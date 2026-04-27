/*
Practica 6 - ENTREGABLE
Programa 1: crea/proyecta fichero.txt y espera a que el Programa 2 lo modifique.
Luego proyecta fichero_tr.txt y copia el contenido modificado de fichero.txt.

Ejecucion en terminal 1:
  ./proyeccion1
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
#include <time.h>
#include <unistd.h>

#define FILE_TXT "fichero.txt"
#define FILE_TR "fichero_tr.txt"
#define TARGET "0000001234abcd"

static int ensure_initial_file(void)
{
    int fd = open(FILE_TXT, O_RDWR | O_CREAT, 0644);
    if (fd < 0)
    {
        perror("open fichero.txt");
        return -1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat fichero.txt");
        close(fd);
        return -1;
    }

    if (st.st_size == 0)
    {
        const char *init = "abcdefghijk";
        size_t n = strlen(init);
        if (write(fd, init, n) != (ssize_t)n)
        {
            perror("write init fichero.txt");
            close(fd);
            return -1;
        }
        if (fsync(fd) < 0)
        {
            perror("fsync init fichero.txt");
            close(fd);
            return -1;
        }
    }

    close(fd);
    return 0;
}

int main(void)
{
    if (ensure_initial_file() < 0)
    {
        return 1;
    }

    int fd = open(FILE_TXT, O_RDWR);
    if (fd < 0)
    {
        perror("open fichero.txt");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) < 0)
    {
        perror("fstat fichero.txt");
        close(fd);
        return 1;
    }

    size_t len = (size_t)st.st_size;
    char *map = (char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (map == MAP_FAILED)
    {
        perror("mmap fichero.txt");
        close(fd);
        return 1;
    }

    printf("[PROY1] fichero.txt proyectado (len=%zu). Contenido actual: %.*s\n",
           len,
           (int)len,
           map);
    printf("[PROY1] Esperando a que proyeccion2 deje: %s\n", TARGET);
    fflush(stdout);

    /* Espera activa corta para ver cambio entre terminales */
    for (int i = 0; i < 300; i++)
    {
        if (len == strlen(TARGET) && strncmp(map, TARGET, len) == 0)
        {
            break;
        }
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000L;
        nanosleep(&ts, NULL);

        struct stat st_new;
        if (fstat(fd, &st_new) == 0 && (size_t)st_new.st_size != len)
        {
            if (munmap(map, len) < 0)
            {
                perror("munmap remap");
                close(fd);
                return 1;
            }
            len = (size_t)st_new.st_size;
            map = (char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (map == MAP_FAILED)
            {
                perror("mmap remap");
                close(fd);
                return 1;
            }
        }
    }

    printf("[PROY1] Contenido final observado en fichero.txt: %.*s\n", (int)len, map);

    int fd_tr = open(FILE_TR, O_RDWR | O_CREAT, 0644);
    if (fd_tr < 0)
    {
        perror("open fichero_tr.txt");
        munmap(map, len);
        close(fd);
        return 1;
    }

    if (ftruncate(fd_tr, (off_t)len) < 0)
    {
        perror("ftruncate fichero_tr.txt");
        close(fd_tr);
        munmap(map, len);
        close(fd);
        return 1;
    }

    char *map_tr = (char *)mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd_tr, 0);
    if (map_tr == MAP_FAILED)
    {
        perror("mmap fichero_tr.txt");
        close(fd_tr);
        munmap(map, len);
        close(fd);
        return 1;
    }

    memcpy(map_tr, map, len);
    if (msync(map_tr, len, MS_SYNC) < 0)
    {
        perror("msync fichero_tr.txt");
    }

    printf("[PROY1] Copiado contenido de fichero.txt a fichero_tr.txt: %.*s\n", (int)len, map_tr);

    munmap(map_tr, len);
    close(fd_tr);
    munmap(map, len);
    close(fd);

    return 0;
}

#endif
