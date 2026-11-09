/*
Binario auxiliar para el punto 4 de la Practica 5.
Se ejecuta tras execl() desde el hijo para observar cambio de imagen.
Calcula coseno de 100 angulos.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa debe ejecutarse en POSIX (Linux/WSL/macOS).\n");
    return 1;
}

#else

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posix_compat.h"

#define PI 3.14159265358979323846

int main(void)
{
    const size_t n = 100;
    double *angles = (double *)malloc(n * sizeof(double));
    if (!angles)
    {
        perror("malloc angles");
        return 1;
    }

    for (size_t i = 0; i < n; i++)
    {
        angles[i] = ((double)i) * 2.0 * PI / (double)n;
    }

    printf("[COS-EXEC] PID=%ld\n", (long)getpid());
    printf("[COS-EXEC] dir(main)=%p, dir(angles)=%p\n", (void *)&main, (void *)angles);

    so_print_maps("MAPA EN IMAGEN NUEVA (post-exec)");

    double acc = 0.0;
    for (size_t i = 0; i < n; i++)
    {
        acc += cos(angles[i]);
    }

    printf("[COS-EXEC] Suma de cosenos (100 angulos) = %.12f\n", acc);

    free(angles);
    return 0;
}

#endif
