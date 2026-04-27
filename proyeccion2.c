/*
Practica 6 - ENTREGABLE
Programa 2: modifica fichero.txt a 0000001234abcd usando buffer temporal,
memset, memcpy y strncat; luego copia completo al mapa compartido.

Ejecucion en terminal 2:
  ./proyeccion2
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa debe ejecutarse en POSIX (Linux/WSL/macOS).\n");
    return 1;
}

#else

#define _XOPEN_SOURCE 700

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define FILE_TXT "fichero.txt"

int main(void)
{
    const char *suffix = "abcd";
    const char *middle = "1234";
    const size_t final_len = 14; /* 0000001234abcd */

    int fd = open(FILE_TXT, O_RDWR);
    if (fd < 0)
    {
        perror("open fichero.txt");
        return 1;
    }

    if (ftruncate(fd, (off_t)final_len) < 0)
    {
        perror("ftruncate fichero.txt");
        close(fd);
        return 1;
    }

    char *map = (char *)mmap(NULL, final_len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        perror("mmap fichero.txt");
        close(fd);
        return 1;
    }

    char buffer[64];
    memset(buffer, 0, sizeof(buffer));

    memset(buffer, '0', 6);
    memcpy(buffer + 6, middle, 4);
    buffer[10] = '\0';
    strncat(buffer, suffix, 4);

    if (strlen(buffer) != final_len)
    {
        fprintf(stderr, "Construccion de string inesperada: %s\n", buffer);
        munmap(map, final_len);
        close(fd);
        return 1;
    }

    memcpy(map, buffer, final_len);

    if (msync(map, final_len, MS_SYNC) < 0)
    {
        perror("msync fichero.txt");
        munmap(map, final_len);
        close(fd);
        return 1;
    }

    printf("[PROY2] Nuevo contenido escrito en fichero.txt: %.*s\n", (int)final_len, map);

    munmap(map, final_len);
    close(fd);

    return 0;
}

#endif
