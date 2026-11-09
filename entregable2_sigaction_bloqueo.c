/*
ENTREGABLE 2 (Punto 6):
- Padre captura SIGUSR1 y SIGUSR2 con sigaction.
- Padre ignora SIGINT.
- H1 envia SIGUSR1, espera 4s, envia SIGUSR2, espera 2s y termina con exit(21).
- H2 envia SIGINT al padre y termina.
- Padre bloquea SIGUSR1 con sigprocmask, duerme para que H1 envie ambas,
  comprueba SIGUSR1 pendiente con sigpending/sigismember y luego desbloquea.
  Debe procesarse SIGUSR2 antes que SIGUSR1.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con sigaction/sigprocmask/fork.\n");
    return 1;
}

#else

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void manejador(int sig)
{
    /* write() es async-signal-safe; printf()/fflush() no lo son. */
    if (sig == SIGUSR1)
    {
        static const char msg[] = "[ENT2-PADRE] Capturada SIGUSR1\n";
        (void)write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    }
    else if (sig == SIGUSR2)
    {
        static const char msg[] = "[ENT2-PADRE] Capturada SIGUSR2\n";
        (void)write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    }
}

int main(void)
{
    struct sigaction sa;
    sigset_t mask_usr1, pending;
    pid_t h1, h2;

    sa.sa_handler = manejador;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGUSR1, &sa, NULL) < 0)
    {
        perror("sigaction SIGUSR1");
        return 1;
    }
    if (sigaction(SIGUSR2, &sa, NULL) < 0)
    {
        perror("sigaction SIGUSR2");
        return 1;
    }

    struct sigaction sa_ignore;
    sa_ignore.sa_handler = SIG_IGN;
    sigemptyset(&sa_ignore.sa_mask);
    sa_ignore.sa_flags = SA_RESTART;
    if (sigaction(SIGINT, &sa_ignore, NULL) < 0)
    {
        perror("sigaction SIGINT");
        return 1;
    }

    sigemptyset(&mask_usr1);
    sigaddset(&mask_usr1, SIGUSR1);
    if (sigprocmask(SIG_BLOCK, &mask_usr1, NULL) < 0)
    {
        perror("sigprocmask bloqueo SIGUSR1");
        return 1;
    }

    printf("[ENT2-PADRE] SIGUSR1 bloqueada antes de crear hijos.\n");
    fflush(stdout);

    h1 = fork();
    if (h1 < 0)
    {
        perror("fork H1");
        return 1;
    }

    if (h1 == 0)
    {
        pid_t p = getppid();
        printf("[ENT2-H1] Envio SIGUSR1 al padre=%ld\n", (long)p);
        fflush(stdout);
        kill(p, SIGUSR1);

        sleep(4);

        printf("[ENT2-H1] Envio SIGUSR2 al padre=%ld\n", (long)p);
        fflush(stdout);
        kill(p, SIGUSR2);

        sleep(2);
        _exit(21);
    }

    h2 = fork();
    if (h2 < 0)
    {
        perror("fork H2");
        kill(h1, SIGKILL);
        waitpid(h1, NULL, 0);
        return 1;
    }

    if (h2 == 0)
    {
        pid_t p = getppid();
        sleep(1);
        printf("[ENT2-H2] Envio SIGINT al padre=%ld (debe ignorarse)\n", (long)p);
        fflush(stdout);
        kill(p, SIGINT);
        _exit(0);
    }

    /* Dormimos para asegurar que H1 emite SIGUSR2 antes de desbloquear SIGUSR1. */
    sleep(6);

    if (sigpending(&pending) < 0)
    {
        perror("sigpending");
        return 1;
    }

    if (sigismember(&pending, SIGUSR1))
    {
        printf("[ENT2-PADRE] Confirmado: SIGUSR1 esta pendiente.\n");
    }
    else
    {
        printf("[ENT2-PADRE] Aviso: SIGUSR1 no aparece pendiente (revisar temporizacion).\n");
    }
    fflush(stdout);

    if (sigprocmask(SIG_UNBLOCK, &mask_usr1, NULL) < 0)
    {
        perror("sigprocmask desbloqueo SIGUSR1");
        return 1;
    }

    printf("[ENT2-PADRE] SIGUSR1 desbloqueada.\n");
    fflush(stdout);

    int status_h1 = 0;
    if (waitpid(h1, &status_h1, 0) < 0)
    {
        perror("waitpid H1");
        return 1;
    }

    if (WIFEXITED(status_h1))
    {
        printf("[ENT2-PADRE] H1 termino con codigo exit=%d\n", WEXITSTATUS(status_h1));
    }

    int status_h2 = 0;
    waitpid(h2, &status_h2, 0);
    printf("[ENT2-PADRE] H2 finalizado. Padre termina normalmente.\n");
    return 0;
}

#endif
