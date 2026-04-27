/*
Enunciado (puntos 1, 2 y 3):
1) Crear un proceso hijo con fork y mostrar informacion identificativa de padre e hijo usando
   getpid, getppid, getuid, geteuid, getgid y getenv.
2) Comprobar que las direcciones de variables globales, locales y dinamicas son las mismas en
   padre e hijo, pero que sus valores pueden divergir tras modificar en un proceso (copy-on-write).
3) Comprobar que un fichero abierto por el padre antes de fork queda compartido por el hijo.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con fork.\n");
    return 1;
}

#else

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static int g_counter = 10;

static void print_identity(const char *tag)
{
    const char *user = getenv("USER");
    const char *home = getenv("HOME");
    const char *shell = getenv("SHELL");

    printf("[%s] pid=%ld ppid=%ld uid=%ld euid=%ld gid=%ld\n",
           tag,
           (long)getpid(),
           (long)getppid(),
           (long)getuid(),
           (long)geteuid(),
           (long)getgid());
    printf("[%s] getenv USER=%s HOME=%s SHELL=%s\n",
           tag,
           user ? user : "(null)",
           home ? home : "(null)",
           shell ? shell : "(null)");
}

int main(void)
{
    int local_counter = 20;
    int *heap_counter = (int *)malloc(sizeof(int));
    if (heap_counter == NULL)
    {
        perror("malloc");
        return 1;
    }
    *heap_counter = 30;

    int fd = open("entregable1_shared.log", O_CREAT | O_TRUNC | O_RDWR, 0644);
    if (fd < 0)
    {
        perror("open entregable1_shared.log");
        free(heap_counter);
        return 1;
    }

    int c2p[2];
    int p2c[2];
    if (pipe(c2p) < 0 || pipe(p2c) < 0)
    {
        perror("pipe");
        close(fd);
        free(heap_counter);
        return 1;
    }

    print_identity("PADRE-ANTES-FORK");
    printf("[PADRE-ANTES-FORK] Direcciones g=%p local=%p heap_ptr=%p\n",
           (void *)&g_counter,
           (void *)&local_counter,
           (void *)heap_counter);
    printf("[PADRE-ANTES-FORK] Valores g=%d local=%d *heap=%d\n",
           g_counter,
           local_counter,
           *heap_counter);

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        close(c2p[0]);
        close(c2p[1]);
        close(p2c[0]);
        close(p2c[1]);
        close(fd);
        free(heap_counter);
        return 1;
    }

    if (pid == 0)
    {
        close(c2p[0]);
        close(p2c[1]);

        print_identity("HIJO");
        printf("[HIJO] Direcciones g=%p local=%p heap_ptr=%p\n",
               (void *)&g_counter,
               (void *)&local_counter,
               (void *)heap_counter);
        printf("[HIJO] Valores iniciales g=%d local=%d *heap=%d\n",
               g_counter,
               local_counter,
               *heap_counter);

        g_counter = 111;
        local_counter = 222;
        *heap_counter = 333;

        printf("[HIJO] Tras modificar: g=%d local=%d *heap=%d\n",
               g_counter,
               local_counter,
               *heap_counter);

        dprintf(fd,
                "HIJO(pid=%ld) escribe primero. offset compartido tras write=%lld\n",
                (long)getpid(),
                (long long)lseek(fd, 0, SEEK_CUR));

        if (write(c2p[1], "X", 1) != 1)
        {
            perror("write c2p");
        }

        char ack = 0;
        if (read(p2c[0], &ack, 1) != 1)
        {
            perror("read p2c");
        }

        close(c2p[1]);
        close(p2c[0]);
        close(fd);
        free(heap_counter);
        _exit(0);
    }

    close(c2p[1]);
    close(p2c[0]);

    char ready = 0;
    if (read(c2p[0], &ready, 1) != 1)
    {
        perror("read c2p");
    }

    printf("[PADRE] Tras cambios del hijo, valores propios siguen: g=%d local=%d *heap=%d\n",
           g_counter,
           local_counter,
           *heap_counter);

    dprintf(fd,
            "PADRE(pid=%ld) escribe despues. offset compartido antes write=%lld\n",
            (long)getpid(),
            (long long)lseek(fd, 0, SEEK_CUR));

    if (write(p2c[1], "Y", 1) != 1)
    {
        perror("write p2c");
    }

    int status = 0;
    waitpid(pid, &status, 0);

    printf("[PADRE] Hijo terminado con estado=%d\n", WIFEXITED(status) ? WEXITSTATUS(status) : -1);
    printf("[PADRE] Revisa entregable1_shared.log para ver escrituras secuenciales en un fd compartido.\n");

    close(c2p[0]);
    close(p2c[1]);
    close(fd);
    free(heap_counter);
    return 0;
}

#endif
