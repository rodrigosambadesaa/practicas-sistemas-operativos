/*
Optativa T2 - Ejercicio 3
Genera N zombies y M huerfanos.

Uso:
  ./opt2_3 <N_zombies> <M_huerfanos>

Notas:
- Los zombies y huerfanos pueden coexistir (en ramas distintas).
- Un huerfano puede acabar como zombie si su nuevo padre temporal no lo espera al terminar.
- Un zombie no llega a ser huerfano porque ya esta terminado.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere POSIX (Linux/WSL/macOS).\n");
    return 1;
}

#else

#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void usage(const char *p)
{
    fprintf(stderr, "Uso: %s <N_zombies> <M_huerfanos>\n", p);
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        usage(argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);
    if (N < 0 || M < 0)
    {
        usage(argv[0]);
        return 1;
    }

    pid_t keeper = fork();
    if (keeper < 0)
    {
        perror("fork keeper");
        return 1;
    }

    if (keeper == 0)
    {
        printf("[OPT2-3 KEEPER] PID=%ld creando %d zombies.\n", (long)getpid(), N);
        fflush(stdout);

        for (int i = 0; i < N; i++)
        {
            pid_t z = fork();
            if (z < 0)
            {
                perror("fork zombie");
                _exit(1);
            }
            if (z == 0)
            {
                _exit(10 + (i % 10));
            }
        }

        printf("[OPT2-3 KEEPER] Zombies creados. No hago wait durante 10s para observarlos.\n");
        fflush(stdout);
        sleep(10);

        int st;
        while (waitpid(-1, &st, 0) > 0)
        {
        }

        _exit(0);
    }

    for (int i = 0; i < M; i++)
    {
        pid_t mid = fork();
        if (mid < 0)
        {
            perror("fork mid");
            continue;
        }

        if (mid == 0)
        {
            pid_t orf = fork();
            if (orf < 0)
            {
                perror("fork orphan");
                _exit(1);
            }
            if (orf == 0)
            {
                sleep(6);
                printf("[OPT2-3 ORFANO] PID=%ld PPID=%ld (debe ser 1/systemd tras quedar huerfano)\n",
                       (long)getpid(),
                       (long)getppid());
                fflush(stdout);
                _exit(0);
            }

            _exit(0);
        }
    }

    int st = 0;
    while (waitpid(-1, &st, 0) > 0)
    {
    }

    waitpid(keeper, NULL, 0);

    printf("[OPT2-3 PADRE] Fin. Resumen teorico:\n");
    printf("  - Si, zombies y huerfanos pueden coexistir al mismo tiempo.\n");
    printf("  - Un huerfano puede terminar y ser zombie temporal bajo su nuevo padre adoptivo.\n");
    printf("  - Un zombie no pasa a huerfano: ya termino y solo espera recoleccion.\n");
    return 0;
}

#endif
