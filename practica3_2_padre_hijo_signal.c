/*
Punto 2:
Padre e hijo se envian SIGUSR1/SIGUSR2 y usan un gestor personalizado.
El padre envia despues SIGTERM al hijo y hace wait.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con fork/kill/signal.\n");
    return 1;
}

#else

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t hijo_pid_global = -1;

static void gestion(int numero_de_senhal)
{
    /* La salida por write evita bloqueos y UB al interrumpir stdio. */
    switch (numero_de_senhal)
    {
    case SIGUSR1:
    {
        static const char msg[] = "[P2] Senhal SIGUSR1 recibida.\n";
        (void)write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        break;
    }
    case SIGUSR2:
    {
        static const char msg[] = "[P2] Senhal SIGUSR2 recibida.\n";
        (void)write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        break;
    }
    case SIGTERM:
    {
        static const char msg[] = "[P2] Senhal SIGTERM recibida; termino.\n";
        (void)write(STDOUT_FILENO, msg, sizeof(msg) - 1);
        _exit(0);
    }
    default:
        break;
    }
}

int main(void)
{
    pid_t padre, hijo;

    if (signal(SIGUSR1, gestion) == SIG_ERR)
    {
        perror("signal SIGUSR1");
        return 1;
    }
    if (signal(SIGUSR2, gestion) == SIG_ERR)
    {
        perror("signal SIGUSR2");
        return 1;
    }
    if (signal(SIGTERM, gestion) == SIG_ERR)
    {
        perror("signal SIGTERM");
        return 1;
    }

    padre = getpid();
    hijo = fork();
    if (hijo < 0)
    {
        perror("fork");
        return 1;
    }

    if (hijo == 0)
    {
        printf("[P2-HIJO] PID=%ld, envio SIGUSR1 al padre=%ld\n", (long)getpid(), (long)padre);
        fflush(stdout);
        kill(padre, SIGUSR1);

        for (;;)
        {
            pause();
        }
    }

    hijo_pid_global = hijo;
    sleep(1);

    printf("[P2-PADRE] PID=%ld, envio SIGUSR2 al hijo=%ld\n", (long)getpid(), (long)hijo);
    fflush(stdout);
    kill(hijo, SIGUSR2);

    sleep(1);

    printf("[P2-PADRE] Envio SIGTERM al hijo=%ld\n", (long)hijo);
    fflush(stdout);
    kill(hijo, SIGTERM);

    int status = 0;
    pid_t who = wait(&status);
    if (who < 0)
    {
        perror("wait");
        return 1;
    }

    if (WIFEXITED(status))
    {
        printf("[P2-PADRE] Hijo %ld finalizado con exit=%d\n", (long)who, WEXITSTATUS(status));
    }
    else if (WIFSIGNALED(status))
    {
        printf("[P2-PADRE] Hijo %ld finalizado por senhal=%d\n", (long)who, WTERMSIG(status));
    }

    return 0;
}

#endif
