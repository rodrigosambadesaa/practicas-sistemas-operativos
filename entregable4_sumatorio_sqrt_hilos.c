/*
ENTREGABLE Practica 4 (Punto 4):
Sumatorio en doble precision de sqrt(j) para j en [0, N), con N = Nc * 1e8 por defecto.
- Nc: numero de cores online.
- Hilo i procesa j tales que j % Nc = i.
- Cada hilo acumula en su variable propia.
- El main junta resultados y compara con version secuencial.

Uso:
  ./entregable4_sumatorio_sqrt_hilos            # usa N = Nc*1e8 (enunciado)
  ./entregable4_sumatorio_sqrt_hilos <N>        # permite N manual para pruebas rapidas
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa debe ejecutarse en POSIX (Linux/WSL/macOS) con pthreads.\n");
    return 1;
}

#else

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include "posix_compat.h"

typedef struct
{
    int tid;
    int nthreads;
    uint64_t N;
    double partial;
} worker_arg_t;

static double now_seconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

static void *worker(void *arg)
{
    worker_arg_t *a = (worker_arg_t *)arg;
    double s = 0.0;

    for (uint64_t j = (uint64_t)a->tid; j < a->N; j += (uint64_t)a->nthreads)
    {
        s += sqrt((double)j);
    }

    a->partial = s;
    pthread_exit(NULL);
}

static uint64_t parse_u64(const char *s)
{
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0')
    {
        fprintf(stderr, "Valor N invalido: %s\n", s);
        exit(1);
    }
    return (uint64_t)v;
}

int main(int argc, char **argv)
{
    long nc_long = so_online_cpus();
    if (nc_long <= 0)
    {
        nc_long = 1;
    }
    int Nc = (int)nc_long;

    uint64_t N_default = (uint64_t)Nc * 100000000ULL;
    uint64_t N = (argc > 1) ? parse_u64(argv[1]) : N_default;

    printf("[P4-ENT] Nc=%d, N=%llu\n", Nc, (unsigned long long)N);

    pthread_t *threads = (pthread_t *)malloc((size_t)Nc * sizeof(pthread_t));
    worker_arg_t *args = (worker_arg_t *)calloc((size_t)Nc, sizeof(worker_arg_t));
    if (!threads || !args)
    {
        fprintf(stderr, "Error de memoria para estructuras de hilos.\n");
        free(threads);
        free(args);
        return 1;
    }

    double t0_par = now_seconds();

    for (int i = 0; i < Nc; i++)
    {
        args[i].tid = i;
        args[i].nthreads = Nc;
        args[i].N = N;
        args[i].partial = 0.0;

        if (pthread_create(&threads[i], NULL, worker, &args[i]) != 0)
        {
            perror("pthread_create");
            free(threads);
            free(args);
            return 1;
        }
    }

    double parallel_sum = 0.0;
    for (int i = 0; i < Nc; i++)
    {
        pthread_join(threads[i], NULL);
        parallel_sum += args[i].partial;
    }

    double t1_par = now_seconds();

    double t0_seq = now_seconds();
    double sequential_sum = 0.0;
    for (uint64_t j = 0; j < N; j++)
    {
        sequential_sum += sqrt((double)j);
    }
    double t1_seq = now_seconds();

    double par_time = t1_par - t0_par;
    double seq_time = t1_seq - t0_seq;
    double speedup = (par_time > 0.0) ? (seq_time / par_time) : 0.0;
    double diff = fabs(parallel_sum - sequential_sum);

    printf("[P4-ENT] Suma paralela   = %.12e\n", parallel_sum);
    printf("[P4-ENT] Suma secuencial = %.12e\n", sequential_sum);
    printf("[P4-ENT] |diferencia|    = %.12e\n", diff);
    printf("[P4-ENT] Tiempo paralelo = %.6f s\n", par_time);
    printf("[P4-ENT] Tiempo secuencial = %.6f s\n", seq_time);
    printf("[P4-ENT] Speedup (seq/par) = %.3f x\n", speedup);

    free(threads);
    free(args);
    return 0;
}

#endif
