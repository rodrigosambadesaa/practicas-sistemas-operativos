/*
Optativa T2 - Ejercicio 2
Dos procesos sin relacion padre-hijo se envian senales entre si.
Funciona en la misma consola o en consolas distintas usando ficheros PID compartidos en /tmp.

Uso en consola A:
  ./opt2_2 canalA A
Uso en consola B:
  ./opt2_2 canalA B
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

#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

static volatile sig_atomic_t got_usr1 = 0;
static volatile sig_atomic_t got_usr2 = 0;

static void on_usr1(int sig)
{
    (void)sig;
    got_usr1 = 1;
}

static void on_usr2(int sig)
{
    (void)sig;
    got_usr2 = 1;
}

static int write_pid_file(const char *path, pid_t pid)
{
    FILE *f = fopen(path, "w");
    if (!f)
    {
        return -1;
    }
    fprintf(f, "%ld\n", (long)pid);
    fclose(f);
    return 0;
}

static int read_pid_file(const char *path, pid_t *pid)
{
    FILE *f = fopen(path, "r");
    if (!f)
    {
        return -1;
    }
    long v = -1;
    if (fscanf(f, "%ld", &v) != 1)
    {
        fclose(f);
        return -1;
    }
    fclose(f);
    *pid = (pid_t)v;
    return 0;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        fprintf(stderr, "Uso: %s <canal> <A|B>\n", argv[0]);
        return 1;
    }

    const char *canal = argv[1];
    const char *rol = argv[2];
    /* No permitir separadores de ruta en los nombres publicados bajo /tmp. */
    size_t canal_len = strlen(canal);
    if (canal_len == 0 || canal_len > 80)
    {
        fprintf(stderr, "Canal vacio o demasiado largo.\n");
        return 1;
    }
    for (size_t i = 0; i < canal_len; ++i)
    {
        unsigned char c = (unsigned char)canal[i];
        if (!isalnum(c) && c != '-' && c != '_')
        {
            fprintf(stderr, "Canal invalido: solo letras, numeros, '-' y '_'.\n");
            return 1;
        }
    }
    if (!(strcmp(rol, "A") == 0 || strcmp(rol, "B") == 0))
    {
        fprintf(stderr, "Rol invalido: %s (usa A o B)\n", rol);
        return 1;
    }

    char mine[256];
    char peer[256];
    snprintf(mine, sizeof(mine), "/tmp/%s.%s.pid", canal, rol);
    snprintf(peer, sizeof(peer), "/tmp/%s.%s.pid", canal, (strcmp(rol, "A") == 0) ? "B" : "A");

    sigset_t blocked, previous, waiting;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGUSR1);
    sigaddset(&blocked, SIGUSR2);
    if (sigprocmask(SIG_BLOCK, &blocked, &previous) < 0)
    {
        perror("sigprocmask");
        return 1;
    }
    waiting = previous;
    sigdelset(&waiting, SIGUSR1);
    sigdelset(&waiting, SIGUSR2);

    if (signal(SIGUSR1, on_usr1) == SIG_ERR || signal(SIGUSR2, on_usr2) == SIG_ERR)
    {
        perror("signal");
        return 1;
    }

    if (write_pid_file(mine, getpid()) < 0)
    {
        perror("write pid file");
        return 1;
    }

    printf("[OPT2-2 %s] PID=%ld publicado en %s\n", rol, (long)getpid(), mine);
    fflush(stdout);

    pid_t peer_pid = -1;
    for (int i = 0; i < 100; i++)
    {
        if (read_pid_file(peer, &peer_pid) == 0 &&
            peer_pid > 0 && (kill(peer_pid, 0) == 0 || errno == EPERM))
        {
            break;
        }
        peer_pid = -1;
        struct timespec ts;
        ts.tv_sec = 0;
        ts.tv_nsec = 100000000L;
        nanosleep(&ts, NULL);
    }

    if (peer_pid <= 0)
    {
        fprintf(stderr, "[OPT2-2 %s] No se encontro PID peer en %s\n", rol, peer);
        return 1;
    }

    printf("[OPT2-2 %s] Detectado peer PID=%ld\n", rol, (long)peer_pid);
    fflush(stdout);

    if (strcmp(rol, "A") == 0)
    {
        printf("[OPT2-2 A] Envio SIGUSR1 a B y espero SIGUSR2 de vuelta...\n");
        fflush(stdout);
        if (kill(peer_pid, SIGUSR1) < 0)
        {
            perror("kill SIGUSR1");
            unlink(mine);
            return 1;
        }

        while (!got_usr2)
        {
            sigsuspend(&waiting);
        }

        printf("[OPT2-2 A] Recibida SIGUSR2 desde B.\n");
    }
    else
    {
        while (!got_usr1)
        {
            sigsuspend(&waiting);
        }
        printf("[OPT2-2 B] Recibida SIGUSR1 desde A; respondo con SIGUSR2.\n");
        fflush(stdout);
        if (kill(peer_pid, SIGUSR2) < 0)
        {
            perror("kill SIGUSR2");
            unlink(mine);
            return 1;
        }
    }

    printf("[OPT2-2 %s] Intercambio completado.\n", rol);
    unlink(mine);
    if (sigprocmask(SIG_SETMASK, &previous, NULL) < 0)
    {
        perror("restaurar mascara");
        return 1;
    }
    return 0;
}

#endif
