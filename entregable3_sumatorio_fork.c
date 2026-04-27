/*
Enunciado (punto 7):
Calcular el sumatorio en doble precision de las raices cuadradas de los primeros 1.000.000
naturales sobre una variable global sum, usando dos procesos hijo para pares e impares.
Ademas, crear un tercer hijo que calcule el sumatorio completo para comparar resultados.

Se implementa solo la opcion que funciona correctamente (opcion d):
- hijo-2 es hijo de hijo-1 (nieto del principal), y hijo-1 completa primero su parte antes de crear hijo-2.
Razon: hijo-2 hereda por fork el valor parcial ya acumulado por hijo-1 y puede completar el total.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con fork.\n");
    return 1;
}

#else

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define N 1000000

double sum = 0.0;

static void compute_even(void)
{
    for (int i = 2; i <= N; i += 2)
    {
        sum += sqrt((double)i);
    }
}

static void compute_odd(void)
{
    for (int i = 1; i <= N; i += 2)
    {
        sum += sqrt((double)i);
    }
}

static double compute_full_local(void)
{
    double total = 0.0;
    for (int i = 1; i <= N; i++)
    {
        total += sqrt((double)i);
    }
    return total;
}

int main(void)
{
    int pipe_h2[2];
    int pipe_h3[2];
    if (pipe(pipe_h2) < 0 || pipe(pipe_h3) < 0)
    {
        perror("pipe");
        return 1;
    }

    pid_t child1 = fork();
    if (child1 < 0)
    {
        perror("fork child1");
        return 1;
    }

    if (child1 == 0)
    {
        close(pipe_h2[0]);
        close(pipe_h3[0]);
        close(pipe_h3[1]);

        compute_even();
        printf("[HIJO-1] Suma parcial pares=%0.12f\n", sum);

        pid_t child2 = fork();
        if (child2 < 0)
        {
            perror("fork child2");
            _exit(1);
        }

        if (child2 == 0)
        {
            compute_odd();
            printf("[HIJO-2] Total heredado+impares=%0.12f\n", sum);
            if (write(pipe_h2[1], &sum, sizeof(sum)) != (ssize_t)sizeof(sum))
            {
                perror("write pipe_h2");
                _exit(2);
            }
            close(pipe_h2[1]);
            _exit(0);
        }

        close(pipe_h2[1]);
        waitpid(child2, NULL, 0);
        _exit(0);
    }

    pid_t child3 = fork();
    if (child3 < 0)
    {
        perror("fork child3");
        return 1;
    }

    if (child3 == 0)
    {
        close(pipe_h3[0]);
        close(pipe_h2[0]);
        close(pipe_h2[1]);

        double full = compute_full_local();
        printf("[HIJO-3] Total completo referencia=%0.12f\n", full);
        if (write(pipe_h3[1], &full, sizeof(full)) != (ssize_t)sizeof(full))
        {
            perror("write pipe_h3");
            _exit(3);
        }
        close(pipe_h3[1]);
        _exit(0);
    }

    close(pipe_h2[1]);
    close(pipe_h3[1]);

    double from_h2 = 0.0;
    double from_h3 = 0.0;

    if (read(pipe_h2[0], &from_h2, sizeof(from_h2)) != (ssize_t)sizeof(from_h2))
    {
        perror("read pipe_h2");
    }
    if (read(pipe_h3[0], &from_h3, sizeof(from_h3)) != (ssize_t)sizeof(from_h3))
    {
        perror("read pipe_h3");
    }

    close(pipe_h2[0]);
    close(pipe_h3[0]);

    waitpid(child1, NULL, 0);
    waitpid(child3, NULL, 0);

    double diff = fabs(from_h2 - from_h3);
    double rel = diff / (fabs(from_h3) > 1.0 ? fabs(from_h3) : 1.0);
    printf("[PADRE] Resultado opcion d (hijo-2) = %0.12f\n", from_h2);
    printf("[PADRE] Resultado referencia (hijo-3) = %0.12f\n", from_h3);
    printf("[PADRE] Diferencia absoluta = %.12e\n", diff);
    printf("[PADRE] Diferencia relativa = %.12e\n", rel);

    if (diff < 1e-5 || rel < 1e-12)
    {
        printf("[PADRE] Coinciden dentro de tolerancia numerica: opcion d valida.\n");
        return 0;
    }

    printf("[PADRE] No coinciden: revisar implementacion.\n");
    return 2;
}

#endif
