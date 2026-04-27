/*
Punto 4:
Proceso padre P crea dos hijos hermanos H1 y H2.
- H2 espera 3 segundos y finaliza a H1 con kill.
- P espera especificamente a H1 con waitpid.
- P y H2 deben terminar normalmente.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con fork/waitpid/kill.\n");
    return 1;
}

#else

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void)
{
    pid_t h1, h2;

    h1 = fork();
    if (h1 < 0)
    {
        perror("fork h1");
        return 1;
    }

    if (h1 == 0)
    {
        printf("[P4-H1] PID=%ld. Entro en bucle esperando senhal.\n", (long)getpid());
        fflush(stdout);
        for (;;)
        {
            pause();
        }
    }

    h2 = fork();
    if (h2 < 0)
    {
        perror("fork h2");
        kill(h1, SIGKILL);
        waitpid(h1, NULL, 0);
        return 1;
    }

    if (h2 == 0)
    {
        printf("[P4-H2] PID=%ld. Espero 3s y envio SIGKILL a H1=%ld\n", (long)getpid(), (long)h1);
        fflush(stdout);
        sleep(3);
        if (kill(h1, SIGKILL) < 0)
        {
            perror("kill H1");
            _exit(1);
        }
        printf("[P4-H2] H1 finalizado. H2 termina normalmente.\n");
        fflush(stdout);
        _exit(0);
    }

    int status_h1 = 0;
    if (waitpid(h1, &status_h1, 0) < 0)
    {
        perror("waitpid H1");
        return 1;
    }

    if (WIFSIGNALED(status_h1))
    {
        printf("[P4-PADRE] H1 termino por senhal=%d\n", WTERMSIG(status_h1));
    }
    else if (WIFEXITED(status_h1))
    {
        printf("[P4-PADRE] H1 termino con exit=%d\n", WEXITSTATUS(status_h1));
    }

    int status_h2 = 0;
    if (waitpid(h2, &status_h2, 0) < 0)
    {
        perror("waitpid H2");
        return 1;
    }

    if (WIFEXITED(status_h2))
    {
        printf("[P4-PADRE] H2 termino normalmente con exit=%d\n", WEXITSTATUS(status_h2));
    }

    printf("[P4-PADRE] Padre termina normalmente.\n");
    return 0;
}

#endif
