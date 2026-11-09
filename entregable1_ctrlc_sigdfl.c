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

#define _POSIX_C_SOURCE 200809L
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    sigset_t blocked, oldmask;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGINT);
    if (sigprocmask(SIG_BLOCK, &blocked, &oldmask) < 0)
    {
        perror("sigprocmask");
        return 1;
    }

    printf("[ENT1] PID=%ld. Pulsa Ctrl+C repetidamente.\n", (long)getpid());
    fflush(stdout);

    for (int i = 1; i <= 8; ++i)
    {
        int sig = 0;
        if (sigwait(&blocked, &sig) != 0)
        {
            fprintf(stderr, "sigwait ha fallado\n");
            return 1;
        }
        printf("[ENT1] Ctrl+C capturado (%d/8).\n", i);
        fflush(stdout);
    }

    puts("[ENT1] Se restaura SIG_DFL. El siguiente Ctrl+C terminara el proceso.");
    fflush(stdout);
    if (signal(SIGINT, SIG_DFL) == SIG_ERR)
    {
        perror("signal SIG_DFL");
        return 1;
    }
    if (sigprocmask(SIG_SETMASK, &oldmask, NULL) < 0)
    {
        perror("restaurar mascara");
        return 1;
    }

    for (;;)
        pause();
}

#endif
