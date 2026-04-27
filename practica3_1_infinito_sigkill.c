/*
Punto 1:
Programa en bucle infinito. Finalizar desde consola con SIGKILL.
Ejemplo:
  ./practica3_1_infinito_sigkill
  kill -9 <pid>
*/

#ifdef _WIN32
#include <stdio.h>

int main(void)
{
    fprintf(stderr, "Este programa requiere un entorno POSIX (Linux/WSL/macOS) con kill/senales UNIX.\n");
    return 1;
}

#else

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

int main(void)
{
    printf("[P1] PID=%ld en bucle infinito. Finaliza con: kill -9 %ld\n", (long)getpid(), (long)getpid());
    fflush(stdout);

    for (;;)
    {
        pause();
    }

    return 0;
}

#endif
