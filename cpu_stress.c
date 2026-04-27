#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <math.h>

int main(int argc, char *argv[])
{
    unsigned long long iterations = 300000000ULL;
    unsigned int report_every = 50000000U;

    if (argc > 1)
    {
        iterations = strtoull(argv[1], NULL, 10);
    }
    if (argc > 2)
    {
        report_every = (unsigned int)strtoul(argv[2], NULL, 10);
        if (report_every == 0)
        {
            report_every = 1;
        }
    }

    volatile double acc = 0.0;
    volatile double x = 0.000001;

    clock_t t0 = clock();

    printf("Iniciando carga de CPU... iteraciones=%llu\n", iterations);

    for (unsigned long long i = 1; i <= iterations; i++)
    {
        double d = (double)i;

        x = x * 1.0000001 + 0.0000003;
        acc += sqrt(d) * 0.000001;
        acc += sin(x) * cos(d * 0.0000001);
        acc -= log(d + 1.0) * 0.00000001;

        if (i % report_every == 0)
        {
            printf("Progreso: %llu / %llu | acc=%f\n", i, iterations, acc);
            fflush(stdout);
        }
    }

    clock_t t1 = clock();
    double elapsed_s = (double)(t1 - t0) / (double)CLOCKS_PER_SEC;

    printf("Finalizado. Tiempo CPU aproximado: %.2f s | acc final=%f\n", elapsed_s, acc);
    return 0;
}
