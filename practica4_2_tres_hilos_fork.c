/*
Punto 2:
Crear 3 hilos. El ultimo crea un hijo con fork.
Comprobar con ps cuantos hilos tiene el hijo y observar su vida cuando termina el hilo creador.
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa debe ejecutarse en POSIX (Linux/WSL/macOS) con pthreads/fork.\n");
    return 1;
}

#else

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static pthread_mutex_t child_mutex = PTHREAD_MUTEX_INITIALIZER;
static pid_t g_child_pid = -1;

static void *simple_thread(void *arg)
{
    long id = (long)arg;
    printf("[P4-2 H%ld] PID=%ld TID=%lu inicio\n", id, (long)getpid(), (unsigned long)pthread_self());
    sleep(2);
    printf("[P4-2 H%ld] fin\n", id);
    pthread_exit(NULL);
}

static void *fork_thread(void *arg)
{
    (void)arg;
    printf("[P4-2 H3] PID=%ld TID=%lu va a hacer fork\n", (long)getpid(), (unsigned long)pthread_self());

    pid_t pid = fork();
    if (pid < 0)
    {
        perror("fork");
        pthread_exit(NULL);
    }

    if (pid == 0)
    {
        printf("[P4-2 HIJO] PID=%ld PPID=%ld. Tras fork desde multihilo, solo existe el hilo llamador.\n",
               (long)getpid(),
               (long)getppid());
        fflush(stdout);

        char cmd[128];
        snprintf(cmd, sizeof(cmd), "ps -T -p %ld -o pid,ppid,tid,comm", (long)getpid());
        int rc = system(cmd);
        if (rc == -1)
        {
            perror("system ps");
        }

        sleep(5);
        printf("[P4-2 HIJO] Sigo vivo aunque el hilo creador del padre haya terminado.\n");
        _exit(0);
    }

    pthread_mutex_lock(&child_mutex);
    g_child_pid = pid;
    pthread_mutex_unlock(&child_mutex);

    printf("[P4-2 H3] fork hecho, hijo PID=%ld. Este hilo terminara ahora.\n", (long)pid);
    pthread_exit(NULL);
}

int main(void)
{
    pthread_t t1, t2, t3;

    if (pthread_create(&t1, NULL, simple_thread, (void *)1L) != 0)
    {
        perror("pthread_create t1");
        return 1;
    }
    if (pthread_create(&t2, NULL, simple_thread, (void *)2L) != 0)
    {
        perror("pthread_create t2");
        return 1;
    }
    if (pthread_create(&t3, NULL, fork_thread, NULL) != 0)
    {
        perror("pthread_create t3");
        return 1;
    }

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);

    pthread_mutex_lock(&child_mutex);
    pid_t child = g_child_pid;
    pthread_mutex_unlock(&child_mutex);

    if (child > 0)
    {
        int status = 0;
        waitpid(child, &status, 0);
        printf("[P4-2 MAIN] Hijo %ld finalizado.\n", (long)child);
    }

    return 0;
}

#endif
