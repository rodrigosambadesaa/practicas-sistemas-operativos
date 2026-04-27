/*
Practica 6 - Punto 1
- open + fstat + lectura caracter a caracter
- mmap de solo lectura, privado, offset 0
- cierre de fd, lectura desde proyeccion y munmap

Uso:
  ./p6_1 <archivo>
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa debe ejecutarse en POSIX (Linux/WSL/macOS).\n");
    return 1;
}

#else

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "Uso: %s <archivo>\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    int fd = open(path, O_RDONLY);
    if (fd < 0)
    {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t len = (size_t)st.st_size;
    printf("[P6-1] Archivo: %s, longitud=%zu bytes\n", path, len);

    printf("[P6-1] Contenido leyendo con read() caracter a caracter:\n");
    if (lseek(fd, 0, SEEK_SET) < 0)
    {
        perror("lseek");
        close(fd);
        return 1;
    }

    for (size_t i = 0; i < len; i++)
    {
        char c;
        ssize_t r = read(fd, &c, 1);
        if (r != 1)
        {
            perror("read");
            close(fd);
            return 1;
        }
        putchar(c);
    }
    putchar('\n');

    char *map = (char *)mmap(NULL, len, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return 1;
    }

    if (close(fd) < 0)
    {
        perror("close");
        munmap(map, len);
        return 1;
    }

    printf("[P6-1] Contenido desde proyeccion mmap (RO, PRIVATE):\n");
    for (size_t i = 0; i < len; i++)
    {
        putchar(map[i]);
    }
    putchar('\n');

    if (munmap(map, len) < 0)
    {
        perror("munmap");
        return 1;
    }

    printf("[P6-1] munmap completado.\n");
    return 0;
}

#endif
