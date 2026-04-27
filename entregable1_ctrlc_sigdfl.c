/*
ENTREGABLE 1 (Punto 3):
Sustituir temporalmente Ctrl+C (SIGINT) por un mensaje.
Tras 8 Ctrl+C, restaurar comportamiento por defecto con SIG_DFL.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con senales UNIX.\n");
    return 1;
}

#else

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

static volatile sig_atomic_t contador_sigint = 0;

static void manejar_sigint(int sig)
{
    (void)sig;
    contador_sigint++;

    printf("[ENT1] Ctrl+C capturado (%d/8).\n", (int)contador_sigint);

    if (contador_sigint >= 8)
    {
        printf("[ENT1] Se restaura accion por defecto de SIGINT. El siguiente Ctrl+C terminara el proceso.\n");
        fflush(stdout);
        signal(SIGINT, SIG_DFL);
        return;
    }

    fflush(stdout);
}

int main(void)
{
    if (signal(SIGINT, manejar_sigint) == SIG_ERR)
    {
        perror("signal SIGINT");
        return 1;
    }

    printf("[ENT1] PID=%ld. Pulsa Ctrl+C repetidamente.\n", (long)getpid());
    fflush(stdout);

    for (;;)
    {
        pause();
    }

    return 0;
}

#endif
