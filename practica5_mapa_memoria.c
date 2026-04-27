/*
Practica 5: Mapa de memoria de un proceso

Este programa implementa los puntos 1..5 del enunciado.
Se recomienda ejecutarlo en terminal interactiva para ir pulsando Enter y observar /proc/PID/maps.

Uso:
  ./p5_main [ruta_binario_cos]

Si no se indica ruta_binario_cos, se usa ./p5_cos_exec
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa debe ejecutarse en POSIX (Linux/WSL/macOS).\n");
    return 1;
}

#else

#define _GNU_SOURCE

#include <errno.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* Variables globales (punto 1) */
static int g_var = 1234;
static int g_arr[16] = {0};

/* Variable global compartida por hilos (punto 5) */
static int g_shared = 100;

typedef struct
{
    int id;
    int *param_ptr;
} thread_arg_t;

static long get_tid(void)
{
    return syscall(SYS_gettid);
}

static void wait_enter(const char *msg)
{
    int c;
    printf("\n--- %s ---\n", msg);
    printf("Pulsa Enter para continuar...\n");
    fflush(stdout);
    do
    {
        c = getchar();
    } while (c != '\n' && c != EOF);
}

static void print_maps(const char *title)
{
    char path[64];
    snprintf(path, sizeof(path), "/proc/%ld/maps", (long)getpid());

    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror("fopen maps");
        return;
    }

    printf("\n========== %s (PID=%ld) ==========\n", title, (long)getpid());

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL)
    {
        fputs(line, stdout);
    }

    fclose(f);
    printf("========== fin maps ==========\n");
    fflush(stdout);
}

static void print_stack_map_for_tid(long tid)
{
    char path[128];
    snprintf(path, sizeof(path), "/proc/%ld/task/%ld/maps", (long)getpid(), tid);

    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror("fopen task maps");
        return;
    }

    printf("[STACK-MAP] TID=%ld archivo=%s\n", tid, path);

    char line[1024];
    while (fgets(line, sizeof(line), f) != NULL)
    {
        if (strstr(line, "[stack") != NULL)
        {
            fputs(line, stdout);
        }
    }

    fclose(f);
}

static void f1(int p)
{
    int local_f1 = 111;
    printf("[f1] dir(f1)=%p, dir(param p)=%p, dir(local_f1)=%p, valor p=%d\n",
           (void *)&f1,
           (void *)&p,
           (void *)&local_f1,
           p);
}

static void f2(int p)
{
    int local_f2 = 222;
    printf("[f2] dir(f2)=%p, dir(param p)=%p, dir(local_f2)=%p, valor p=%d\n",
           (void *)&f2,
           (void *)&p,
           (void *)&local_f2,
           p);
}

static void *thread_fn(void *arg)
{
    thread_arg_t *a = (thread_arg_t *)arg;
    int local_thread = g_shared + *(a->param_ptr);
    long tid = get_tid();

    printf("[HILO %d] PID=%ld TID=%ld\n", a->id, (long)getpid(), tid);
    printf("[HILO %d] dir(g_shared)=%p valor=%d\n", a->id, (void *)&g_shared, g_shared);
    printf("[HILO %d] dir(parametro recibido)=%p valor=%d\n",
           a->id,
           (void *)a->param_ptr,
           *(a->param_ptr));
    printf("[HILO %d] dir(local_thread)=%p valor=%d\n",
           a->id,
           (void *)&local_thread,
           local_thread);

    print_stack_map_for_tid(tid);

    return NULL;
}

int main(int argc, char **argv)
{
    const char *cos_exec = (argc > 1) ? argv[1] : "./p5_cos_exec";

    int local_var = 5678;
    int local_arr[16] = {0};

    printf("[P5] PID=%ld\n", (long)getpid());

    wait_enter("Punto 1: direcciones de globales/locales/main y mapa inicial");
    printf("[P1] dir(main)=%p\n", (void *)&main);
    printf("[P1] dir(g_var)=%p, dir(g_arr)=%p\n", (void *)&g_var, (void *)g_arr);
    printf("[P1] dir(local_var)=%p, dir(local_arr)=%p\n", (void *)&local_var, (void *)local_arr);
    print_maps("MAPA EN PUNTO 1");

    wait_enter("Punto 2: funciones f1 y f2 con parametro/local");
    f1(11);
    f2(22);
    print_maps("MAPA EN PUNTO 2");

    wait_enter("Punto 3: malloc/free y mapa antes/despues");
    print_maps("ANTES DE malloc");

    size_t small_sz = 1024;
    size_t big_sz = 16 * 1024 * 1024;

    void *small = malloc(small_sz);
    void *big = malloc(big_sz);
    if (!small || !big)
    {
        fprintf(stderr, "malloc fallo: small=%p big=%p\n", small, big);
        free(small);
        free(big);
        return 1;
    }

    memset(small, 0xAA, small_sz);
    memset(big, 0xBB, big_sz);

    printf("[P3] small malloc(%zu) -> %p\n", small_sz, small);
    printf("[P3] big   malloc(%zu) -> %p\n", big_sz, big);
    print_maps("DESPUES DE malloc");

    free(small);
    free(big);
    printf("[P3] free(small) y free(big) ejecutados.\n");
    print_maps("DESPUES DE free");

    wait_enter("Punto 4: fork + malloc en hijo + exec a calculo de cosenos");
    pid_t child = fork();
    if (child < 0)
    {
        perror("fork");
        return 1;
    }

    if (child == 0)
    {
        printf("[P4-HIJO] PID=%ld PPID=%ld\n", (long)getpid(), (long)getppid());
        print_maps("HIJO ANTES DE malloc");

        void *child_mem = malloc(8 * 1024 * 1024);
        if (!child_mem)
        {
            perror("malloc hijo");
            _exit(1);
        }
        memset(child_mem, 0xCC, 8 * 1024 * 1024);
        printf("[P4-HIJO] malloc(8MB) -> %p\n", child_mem);
        print_maps("HIJO DESPUES DE malloc (antes de exec)");

        execl(cos_exec, cos_exec, (char *)NULL);
        perror("execl cos_exec");
        _exit(127);
    }
    else
    {
        int st = 0;
        waitpid(child, &st, 0);
        if (WIFEXITED(st))
        {
            printf("[P4-PADRE] hijo termina con exit=%d\n", WEXITSTATUS(st));
        }
        else if (WIFSIGNALED(st))
        {
            printf("[P4-PADRE] hijo termina por senhal=%d\n", WTERMSIG(st));
        }
    }

    wait_enter("Punto 5 (ENTREGABLE): dos hilos y localizacion de pilas");

    int main_param1 = 7;
    int main_param2 = 9;
    pthread_t t1, t2;
    thread_arg_t a1 = {.id = 1, .param_ptr = &main_param1};
    thread_arg_t a2 = {.id = 2, .param_ptr = &main_param2};

    printf("[P5-MAIN] dir(g_shared)=%p valor=%d\n", (void *)&g_shared, g_shared);
    printf("[P5-MAIN] dir(main_param1)=%p valor=%d\n", (void *)&main_param1, main_param1);
    printf("[P5-MAIN] dir(main_param2)=%p valor=%d\n", (void *)&main_param2, main_param2);
    print_stack_map_for_tid(get_tid());

    if (pthread_create(&t1, NULL, thread_fn, &a1) != 0)
    {
        perror("pthread_create t1");
        return 1;
    }
    if (pthread_create(&t2, NULL, thread_fn, &a2) != 0)
    {
        perror("pthread_create t2");
        return 1;
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    print_maps("MAPA FINAL EN PADRE");

    printf("\n[P5] Practica 5 completada.\n");
    return 0;
}

#endif
