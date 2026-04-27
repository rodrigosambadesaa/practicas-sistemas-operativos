/*
Punto 5:
Padre crea hijo, espera 5 segundos y envia SIGUSR1.
Hijo ejecuta pause() y se desbloquea al recibir SIGUSR1 capturada.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con fork/pause/senales UNIX.\n");
    return 1;
}

#else

#include <signal.h>
#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

static volatile sig_atomic_t recibida = 0;

static void manejar_usr1(int sig)
{
    (void)sig;
    recibida = 1;
    printf("[P5-HIJO] Recibida SIGUSR1, se desbloquea pause().\n");
    fflush(stdout);
}

int main(void)
{
    pid_t hijo;

    hijo = fork();
    if (hijo < 0)
    {
        perror("fork");
        return 1;
    }

    if (hijo == 0)
    {
        if (signal(SIGUSR1, manejar_usr1) == SIG_ERR)
        {
            perror("signal SIGUSR1");
            _exit(1);
        }

        printf("[P5-HIJO] PID=%ld. Entro en pause().\n", (long)getpid());
        fflush(stdout);
        pause();

        if (recibida)
        {
            printf("[P5-HIJO] Continuo ejecucion tras pause.\n");
        }
        _exit(0);
    }

    printf("[P5-PADRE] PID=%ld. Espero 5s y envio SIGUSR1 a hijo=%ld\n", (long)getpid(), (long)hijo);
    fflush(stdout);
    sleep(5);

    if (kill(hijo, SIGUSR1) < 0)
    {
        perror("kill SIGUSR1");
        return 1;
    }

    int status = 0;
    waitpid(hijo, &status, 0);
    printf("[P5-PADRE] Hijo finalizado.\n");
    return 0;
}

#endif
