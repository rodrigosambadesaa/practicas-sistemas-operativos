/*
Enunciado (puntos 4, 5 y 6):
4) Generar dos hijos: uno llega pronto a zombie y el padre, tras un tiempo, ejecuta wait y termina.
   El otro hijo sigue activo mas tiempo y queda huerfano.
5) Al final de uno de los hijos, cambiar su imagen con exec* para ejecutar ls seguido de un printf,
   comprobando que el printf no se ejecuta.
6) Comprobar que ocurre con un zombie si su padre acaba sin ejecutar wait.

Uso:
- ./entregable2_zombie_orfano_exec wait
- ./entregable2_zombie_orfano_exec no-wait
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con fork/ps/exec.\n");
    return 1;
}

#else

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

static void show_ps(pid_t ppid, pid_t zpid, pid_t opid)
{
    char cmd[256];
    snprintf(cmd,
             sizeof(cmd),
             "ps -o pid,ppid,state,comm -p %ld,%ld,%ld",
             (long)ppid,
             (long)zpid,
             (long)opid);
    printf("[PADRE] Ejecutando: %s\n", cmd);
    fflush(stdout);
    int rc = system(cmd);
    if (rc == -1)
    {
        perror("system ps");
    }
}

int main(int argc, char *argv[])
{
    const int no_wait_mode = (argc > 1 && strcmp(argv[1], "no-wait") == 0);
    printf("[PADRE] PID=%ld modo=%s\n", (long)getpid(), no_wait_mode ? "no-wait" : "wait");

    pid_t zombie_child = fork();
    if (zombie_child < 0)
    {
        perror("fork zombie_child");
        return 1;
    }
    if (zombie_child == 0)
    {
        printf("[HIJO-ZOMBIE] PID=%ld PPID=%ld -> salgo rapido con exit(42)\n",
               (long)getpid(),
               (long)getppid());
        _exit(42);
    }

    pid_t orphan_child = fork();
    if (orphan_child < 0)
    {
        perror("fork orphan_child");
        return 1;
    }
    if (orphan_child == 0)
    {
        printf("[HIJO-ORFANO] PID=%ld PPID=%ld -> duermo para sobrevivir al padre\n",
               (long)getpid(),
               (long)getppid());
        sleep(6);

        printf("[HIJO-ORFANO] Cambio de imagen con execlp(ls).\n");
        fflush(stdout);
        execlp("ls", "ls", "-l", (char *)NULL);

        perror("execlp ls");
        printf("[HIJO-ORFANO] Este printf solo aparece si exec falla.\n");
        _exit(127);
    }

    sleep(2);
    show_ps(getpid(), zombie_child, orphan_child);

    if (!no_wait_mode)
    {
        int status = 0;
        pid_t who = wait(&status);
        if (who < 0)
        {
            perror("wait");
        }
        else if (WIFEXITED(status))
        {
            printf("[PADRE] wait recogio hijo pid=%ld con exit=%d\n",
                   (long)who,
                   WEXITSTATUS(status));
        }
        else
        {
            printf("[PADRE] wait recogio hijo pid=%ld sin salida normal\n", (long)who);
        }
    }
    else
    {
        printf("[PADRE] modo no-wait: termino sin esperar, para observar que pasa con zombie.\n");
    }

    printf("[PADRE] Fin del padre. El hijo largo debe quedar huerfano temporalmente.\n");
    return 0;
}

#endif
