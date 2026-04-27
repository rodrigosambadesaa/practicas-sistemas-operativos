/*
Punto 1:
Comprobar acceso compartido a variables globales y a variables de otros hilos via punteros.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa debe ejecutarse en POSIX (Linux/WSL/macOS) con pthreads.\n");
    return 1;
}

#else

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static int g_global = 10;

typedef struct
{
    int id;
    int *shared_main_local;
} thread_arg_t;

static void *worker(void *arg)
{
    thread_arg_t *a = (thread_arg_t *)arg;
    int local_hilo = 1000 + a->id;

    printf("[P4-1 H%d] Inicio: global=%d, main_local=%d, local_hilo=%d\n",
           a->id,
           g_global,
           *(a->shared_main_local),
           local_hilo);

    if (a->id == 0)
    {
        g_global += 5;
        *(a->shared_main_local) += 7;
        local_hilo += 3;
        printf("[P4-1 H%d] Modifico global y main_local. local_hilo=%d\n", a->id, local_hilo);
    }

    sleep(1);

    printf("[P4-1 H%d] Tras cambios: global=%d, main_local=%d, local_hilo=%d\n",
           a->id,
           g_global,
           *(a->shared_main_local),
           local_hilo);

    pthread_exit(NULL);
}

int main(void)
{
    pthread_t hilos[3];
    thread_arg_t args[3];
    int main_local = 20;

    printf("[P4-1 MAIN] Inicio: global=%d, main_local=%d\n", g_global, main_local);

    for (int i = 0; i < 3; i++)
    {
        args[i].id = i;
        args[i].shared_main_local = &main_local;
        if (pthread_create(&hilos[i], NULL, worker, &args[i]) != 0)
        {
            perror("pthread_create");
            return 1;
        }
    }

    for (int i = 0; i < 3; i++)
    {
        pthread_join(hilos[i], NULL);
    }

    printf("[P4-1 MAIN] Final: global=%d, main_local=%d\n", g_global, main_local);
    return 0;
}

#endif
