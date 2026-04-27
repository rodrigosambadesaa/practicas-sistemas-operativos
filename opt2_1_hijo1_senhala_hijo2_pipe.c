/*
Optativa T2 - Ejercicio 1
Solucion: el padre comunica el PID del segundo hijo al primero mediante un pipe.

Uso:
  ./opt2_1
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere POSIX (Linux/WSL/macOS).\n");
    return 1;
}

#else

#define _POSIX_C_SOURCE 200809L

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t got_usr1 = 0;

static void on_usr1(int sig)
{
    (void)sig;
    got_usr1 = 1;
}

int main(void)
{
    int pfd[2] = {-1, -1};
    if (pipe(pfd) < 0)
    {
        perror("pipe");
        return 1;
    }

    pid_t h1 = fork();
    if (h1 < 0)
    {
        perror("fork h1");
        return 1;
    }

    if (h1 == 0)
    {
        close(pfd[1]);
        pid_t h2_pid = -1;

        ssize_t r = read(pfd[0], &h2_pid, sizeof(h2_pid));
        if (r != (ssize_t)sizeof(h2_pid))
        {
            perror("h1 read pid h2");
            _exit(1);
        }

        printf("[OPT2-1 H1] Recibido PID de H2=%ld. Envio SIGUSR1.\n", (long)h2_pid);
        fflush(stdout);

        if (kill(h2_pid, SIGUSR1) < 0)
        {
            perror("h1 kill SIGUSR1");
            _exit(1);
        }

        _exit(0);
    }

    pid_t h2 = fork();
    if (h2 < 0)
    {
        perror("fork h2");
        return 1;
    }

    if (h2 == 0)
    {
        close(pfd[0]);
        close(pfd[1]);

        if (signal(SIGUSR1, on_usr1) == SIG_ERR)
        {
            perror("signal SIGUSR1");
            _exit(1);
        }

        printf("[OPT2-1 H2] PID=%ld esperando SIGUSR1 de H1...\n", (long)getpid());
        fflush(stdout);

        while (!got_usr1)
        {
            pause();
        }

        printf("[OPT2-1 H2] SIGUSR1 recibida correctamente.\n");
        fflush(stdout);
        _exit(0);
    }

    close(pfd[0]);
    if (write(pfd[1], &h2, sizeof(h2)) != (ssize_t)sizeof(h2))
    {
        perror("padre write pid h2");
        close(pfd[1]);
        return 1;
    }
    close(pfd[1]);

    int st = 0;
    waitpid(h1, &st, 0);
    waitpid(h2, &st, 0);

    printf("[OPT2-1 PADRE] Fin correcto.\n");
    return 0;
}

#endif
