/*
Practica 7: Sistema de archivos

Uso:
  ./archivos <ruta_relativa_prueba> <permisos_octal_4_digitos>
Ejemplo:
  ./archivos practica7/prueba 0666
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

#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static int starts_with_lower_vowel(const char *name)
{
    if (!name || !name[0])
    {
        return 0;
    }
    char c = (char)tolower((unsigned char)name[0]);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u') && islower((unsigned char)name[0]);
}

static int write_all(int fd, const void *buf, size_t len)
{
    const char *p = (const char *)buf;
    size_t left = len;
    while (left > 0)
    {
        ssize_t w = write(fd, p, left);
        if (w < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            return -1;
        }
        p += (size_t)w;
        left -= (size_t)w;
    }
    return 0;
}

static int parse_octal_perms(const char *s, mode_t *out)
{
    if (!s || strlen(s) != 4)
    {
        return -1;
    }

    char *end = NULL;
    errno = 0;
    long v = strtol(s, &end, 8);
    if (errno != 0 || end == s || *end != '\0' || v < 0 || v > 07777)
    {
        return -1;
    }

    *out = (mode_t)v;
    return 0;
}

static const char *file_type_str(mode_t mode)
{
    if (S_ISREG(mode))
        return "regular";
    if (S_ISDIR(mode))
        return "directorio";
    if (S_ISLNK(mode))
        return "enlace";
    if (S_ISCHR(mode))
        return "char";
    if (S_ISBLK(mode))
        return "block";
    if (S_ISFIFO(mode))
        return "fifo";
    if (S_ISSOCK(mode))
        return "socket";
    return "desconocido";
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <ruta_relativa_prueba> <permisos_octal_4_digitos>\n", argv[0]);
        return 1;
    }

    const char *ruta_prueba = argv[1];
    mode_t nuevos_permisos = 0;
    if (parse_octal_perms(argv[2], &nuevos_permisos) != 0)
    {
        fprintf(stderr, "Error: permisos invalidos '%s'. Deben ser 4 digitos octales (ej. 0666).\n", argv[2]);
        return 1;
    }

    /* 1) Crear fichero.txt con 0664 y escribir segun enunciado. */
    const char seq_num[] = "01234567";
    const char seq_abc[] = "ABCDEFGH";

    int fd1 = open("fichero.txt", O_CREAT | O_TRUNC | O_RDWR, 0664);
    if (fd1 < 0)
    {
        perror("open fichero.txt");
        return 1;
    }

    if (write_all(fd1, seq_num, 8) < 0 || write_all(fd1, seq_abc, 8) < 0)
    {
        perror("write fichero.txt paso inicial");
        close(fd1);
        return 1;
    }

    if (write_all(fd1, seq_num, 8) < 0)
    {
        perror("write fichero.txt segunda secuencia numerica");
        close(fd1);
        return 1;
    }

    /* Ajuste para obtener exactamente: 01234567ABCDEFGH01234ABCDEFGH567 */
    if (lseek(fd1, 21, SEEK_SET) < 0)
    {
        perror("lseek fichero.txt");
        close(fd1);
        return 1;
    }

    if (write_all(fd1, seq_abc, 8) < 0)
    {
        perror("write fichero.txt secuencia alfabetica superpuesta");
        close(fd1);
        return 1;
    }

    if (lseek(fd1, 0, SEEK_END) < 0)
    {
        perror("lseek SEEK_END fichero.txt");
        close(fd1);
        return 1;
    }

    if (write_all(fd1, seq_num + 5, 3) < 0)
    {
        perror("write sufijo fichero.txt");
        close(fd1);
        return 1;
    }

    if (close(fd1) < 0)
    {
        perror("close fichero.txt");
        return 1;
    }

    /* 2) Crear fichero_2.txt (0640), copiar en bloques de 8 y newline. */
    int in = open("fichero.txt", O_RDONLY);
    if (in < 0)
    {
        perror("open fichero.txt lectura");
        return 1;
    }

    int out = open("fichero_2.txt", O_CREAT | O_TRUNC | O_WRONLY, 0640);
    if (out < 0)
    {
        perror("open fichero_2.txt");
        close(in);
        return 1;
    }

    char buf8[8];
    for (;;)
    {
        ssize_t r = read(in, buf8, sizeof(buf8));
        if (r < 0)
        {
            if (errno == EINTR)
            {
                continue;
            }
            perror("read fichero.txt");
            close(in);
            close(out);
            return 1;
        }
        if (r == 0)
        {
            break;
        }

        if (write_all(out, buf8, (size_t)r) < 0 || write_all(out, "\n", 1) < 0)
        {
            perror("write fichero_2.txt");
            close(in);
            close(out);
            return 1;
        }
    }

    close(in);
    close(out);

    if (chmod("fichero.txt", 0760) < 0)
    {
        perror("chmod fichero.txt 0760");
        return 1;
    }

    /* 3) Mostrar nombres de archivos del directorio de trabajo. */
    char cwd[4096];
    if (!getcwd(cwd, sizeof(cwd)))
    {
        perror("getcwd");
        return 1;
    }

    printf("\n[3] Directorio de trabajo: %s\n", cwd);
    DIR *dwork = opendir(cwd);
    if (!dwork)
    {
        perror("opendir cwd");
        return 1;
    }

    printf("[3] Entradas en directorio de trabajo:\n");
    struct dirent *de = NULL;
    while ((de = readdir(dwork)) != NULL)
    {
        printf("  - %s\n", de->d_name);
    }
    closedir(dwork);

    /* 4) Buscar en ruta_prueba archivos regulares que empiecen por vocal minuscula. */
    DIR *dprueba = opendir(ruta_prueba);
    if (!dprueba)
    {
        perror("opendir ruta_prueba");
        return 1;
    }

    printf("\n[4] Archivos en %s que empiezan por vocal minuscula:\n", ruta_prueba);
    unsigned long long total_bytes = 0ULL;

    while ((de = readdir(dprueba)) != NULL)
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            continue;
        }

        char relpath[4096];
        int n = snprintf(relpath, sizeof(relpath), "%s/%s", ruta_prueba, de->d_name);
        if (n < 0 || (size_t)n >= sizeof(relpath))
        {
            fprintf(stderr, "Ruta demasiado larga, se omite: %s/%s\n", ruta_prueba, de->d_name);
            continue;
        }

        struct stat st;
        if (lstat(relpath, &st) < 0)
        {
            perror("lstat entrada ruta_prueba");
            continue;
        }

        const char *tipo = file_type_str(st.st_mode);
        if (!S_ISREG(st.st_mode))
        {
            continue;
        }

        if (!starts_with_lower_vowel(de->d_name))
        {
            continue;
        }

        char *ct = ctime(&st.st_mtime);
        if (!ct)
        {
            ct = "(hora no disponible)\n";
        }

        printf("  %s\n", relpath);
        printf("    tipo: %s\n", tipo);
        printf("    ultima modificacion: %s", ct);
        printf("    inode: %lu\n", (unsigned long)st.st_ino);
        printf("    bloques asignados: %lld\n", (long long)st.st_blocks);
        printf("    espacio ocupado (bytes): %lld\n", (long long)st.st_size);

        total_bytes += (unsigned long long)st.st_size;
    }

    closedir(dprueba);
    printf("  Suma total de espacio (bytes): %llu\n", total_bytes);

    /* 5) Cambiar permisos de archivos regulares en ruta_prueba. */
    dprueba = opendir(ruta_prueba);
    if (!dprueba)
    {
        perror("opendir ruta_prueba para chmod");
        return 1;
    }

    printf("\n[5] Cambio de permisos en archivos regulares de %s:\n", ruta_prueba);
    while ((de = readdir(dprueba)) != NULL)
    {
        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
        {
            continue;
        }

        char relpath[4096];
        int n = snprintf(relpath, sizeof(relpath), "%s/%s", ruta_prueba, de->d_name);
        if (n < 0 || (size_t)n >= sizeof(relpath))
        {
            fprintf(stderr, "Ruta demasiado larga, se omite: %s/%s\n", ruta_prueba, de->d_name);
            continue;
        }

        struct stat st;
        if (lstat(relpath, &st) < 0)
        {
            perror("lstat para chmod");
            continue;
        }

        if (!S_ISREG(st.st_mode))
        {
            continue;
        }

        mode_t oldp = st.st_mode & 07777;
        if (chmod(relpath, nuevos_permisos) < 0)
        {
            perror("chmod archivo regular");
            continue;
        }

        struct stat st_after;
        if (lstat(relpath, &st_after) < 0)
        {
            perror("lstat tras chmod");
            continue;
        }

        mode_t newp = st_after.st_mode & 07777;
        printf("%s: %04o -> %04o\n", de->d_name, (unsigned)oldp, (unsigned)newp);
    }

    closedir(dprueba);

    return 0;
}

#endif
