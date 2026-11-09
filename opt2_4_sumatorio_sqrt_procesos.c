/*
Optativa T2 - Ejercicio 4
Version por procesos del ejercicio 4 de practica 4.
Usa tantos procesos como cores online.

Uso:
  ./opt2_4                # N = Nc * 1e8 (enunciado)
  ./opt2_4 <N>            # para pruebas rapidas
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere POSIX (Linux/WSL/macOS).\n");
    return 1;
}

#else

#define _XOPEN_SOURCE 700
#if defined(__linux__)
#define _DEFAULT_SOURCE 1
#elif defined(__APPLE__)
#define _DARWIN_C_SOURCE 1
#endif

#include <errno.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include "posix_compat.h"

static double now_seconds(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)tv.tv_sec + (double)tv.tv_usec / 1e6;
}

static uint64_t parse_u64(const char *s)
{
    errno = 0;
    char *end = NULL;
    unsigned long long v = strtoull(s, &end, 10);
    if (errno != 0 || end == s || *end != '\0')
    {
        fprintf(stderr, "N invalido: %s\n", s);
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

    double *partials = (double *)mmap(NULL,
                                      (size_t)Nc * sizeof(double),
                                      PROT_READ | PROT_WRITE,
                                      MAP_SHARED | MAP_ANON,
                                      -1,
                                      0);
    if (partials == MAP_FAILED)
    {
        perror("mmap partials");
        return 1;
    }

    memset(partials, 0, (size_t)Nc * sizeof(double));

    printf("[OPT2-4] Nc=%d, N=%llu\n", Nc, (unsigned long long)N);

    double t0_par = now_seconds();

    for (int i = 0; i < Nc; i++)
    {
        pid_t p = fork();
        if (p < 0)
        {
            perror("fork worker");
            munmap(partials, (size_t)Nc * sizeof(double));
            return 1;
        }

        if (p == 0)
        {
            double s = 0.0;
            for (uint64_t j = (uint64_t)i; j < N; j += (uint64_t)Nc)
            {
                s += sqrt((double)j);
            }
            partials[i] = s;
            _exit(0);
        }
    }

    for (int i = 0; i < Nc; i++)
    {
        wait(NULL);
    }

    double parallel_sum = 0.0;
    for (int i = 0; i < Nc; i++)
    {
        parallel_sum += partials[i];
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

    printf("[OPT2-4] Suma paralela   = %.12e\n", parallel_sum);
    printf("[OPT2-4] Suma secuencial = %.12e\n", sequential_sum);
    printf("[OPT2-4] |diferencia|    = %.12e\n", diff);
    printf("[OPT2-4] Tiempo paralelo   = %.6f s\n", par_time);
    printf("[OPT2-4] Tiempo secuencial = %.6f s\n", seq_time);
    printf("[OPT2-4] Speedup (seq/par) = %.3f x\n", speedup);

    munmap(partials, (size_t)Nc * sizeof(double));
    return 0;
}

#endif
