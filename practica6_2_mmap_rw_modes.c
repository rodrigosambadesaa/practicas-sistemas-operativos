/*
Practica 6 - Puntos 2, 3, 4 y 5
- mmap en modo compartido o privado
- permisos ro o rw
- comparacion de mapa de memoria antes/despues
- uso de msync para forzar escritura inmediata en modo compartido

Uso:
  ./p6_2 <archivo> <shared|private> <ro|rw> [texto]

Ejemplos:
  ./p6_2 fichero.txt shared rw XYZ
  ./p6_2 fichero.txt private rw XYZ
  ./p6_2 fichero.txt shared ro
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa debe ejecutarse en POSIX (Linux/WSL/macOS).\n");
    return 1;
}

#else

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "posix_compat.h"

static int is_shared(const char *s)
{
    return strcmp(s, "shared") == 0;
}

static int is_private(const char *s)
{
    return strcmp(s, "private") == 0;
}

static int is_ro(const char *s)
{
    return strcmp(s, "ro") == 0;
}

static int is_rw(const char *s)
{
    return strcmp(s, "rw") == 0;
}

int main(int argc, char **argv)
{
    if (argc < 4)
    {
        fprintf(stderr, "Uso: %s <archivo> <shared|private> <ro|rw> [texto]\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];
    const char *map_mode = argv[2];
    const char *perm_mode = argv[3];
    const char *text = (argc >= 5) ? argv[4] : "Z";

    if ((!is_shared(map_mode) && !is_private(map_mode)) || (!is_ro(perm_mode) && !is_rw(perm_mode)))
    {
        fprintf(stderr, "Parametros invalidos.\n");
        return 1;
    }

    int open_flags = is_ro(perm_mode) ? O_RDONLY : O_RDWR;
    int fd = open(path, open_flags);
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
    int prot = is_ro(perm_mode) ? PROT_READ : (PROT_READ | PROT_WRITE);
    int flags = is_shared(map_mode) ? MAP_SHARED : MAP_PRIVATE;

    printf("[P6-2] archivo=%s len=%zu modo=%s perm=%s\n", path, len, map_mode, perm_mode);
    so_print_maps("ANTES DE mmap");

    char *map = (char *)mmap(NULL, len, prot, flags, fd, 0);
    if (map == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return 1;
    }

    so_print_maps("DESPUES DE mmap");

    printf("[P6-2] Primeros bytes previos en mapa: ");
    size_t preview = (len < 32) ? len : 32;
    for (size_t i = 0; i < preview; i++)
    {
        putchar(map[i]);
    }
    putchar('\n');

    if (is_rw(perm_mode))
    {
        size_t n = strlen(text);
        if (n > len)
        {
            n = len;
        }

        memcpy(map, text, n);
        printf("[P6-2] Modificados %zu bytes en la proyeccion.\n", n);

        if (is_shared(map_mode))
        {
            if (msync(map, len, MS_SYNC) < 0)
            {
                perror("msync");
            }
            else
            {
                printf("[P6-2] msync(MS_SYNC) completado.\n");
            }
        }
        else
        {
            printf("[P6-2] MAP_PRIVATE: los cambios son privados (copy-on-write) y no deben persistir en el archivo.\n");
        }
    }
    else
    {
        printf("[P6-2] Modo solo lectura: no se modifica el mapa.\n");
    }

    if (munmap(map, len) < 0)
    {
        perror("munmap");
        close(fd);
        return 1;
    }

    close(fd);

    int fd2 = open(path, O_RDONLY);
    if (fd2 < 0)
    {
        perror("open verificacion");
        return 1;
    }

    struct stat st2;
    if (fstat(fd2, &st2) < 0)
    {
        perror("fstat verificacion");
        close(fd2);
        return 1;
    }

    size_t len2 = (size_t)st2.st_size;
    char *buf = (char *)malloc(len2 + 1);
    if (!buf)
    {
        fprintf(stderr, "malloc fallo\n");
        close(fd2);
        return 1;
    }

    ssize_t rr = read(fd2, buf, len2);
    if (rr < 0)
    {
        perror("read verificacion");
        free(buf);
        close(fd2);
        return 1;
    }
    buf[rr] = '\0';
    close(fd2);

    printf("[P6-2] Contenido actual del archivo tras munmap/msync: %s\n", buf);
    free(buf);

    return 0;
}

#endif
