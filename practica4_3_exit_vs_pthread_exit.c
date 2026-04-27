/*
Punto 3:
Un hilo ejecuta exit() antes que otros terminen.
Se comprueba que exit() termina todo el proceso (todos los hilos).
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

static void *thread_largo(void *arg)
{
    long id = (long)arg;
    for (int i = 1; i <= 10; i++)
    {
        printf("[P4-3 H%ld] iteracion %d\n", id, i);
        fflush(stdout);
        sleep(1);
    }
    pthread_exit(NULL);
}

static void *thread_exit_global(void *arg)
{
    (void)arg;
    sleep(2);
    printf("[P4-3 HEXIT] Llamo a exit(7). Esto termina el proceso completo.\n");
    fflush(stdout);
    exit(7);
}

int main(void)
{
    pthread_t t1, t2, t3;

    pthread_create(&t1, NULL, thread_largo, (void *)1L);
    pthread_create(&t2, NULL, thread_largo, (void *)2L);
    pthread_create(&t3, NULL, thread_exit_global, NULL);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    printf("[P4-3 MAIN] Este mensaje no deberia aparecer.\n");
    return 0;
}

#endif
