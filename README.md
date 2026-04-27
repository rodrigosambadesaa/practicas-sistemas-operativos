# Practica 1 - Consumo de memoria y CPU en C

## Archivos

- `memory_stress.c`: consumo progresivo de memoria usando `malloc`, escritura de datos y pausa con `sleep`.
- `cpu_stress.c`: carga alta de CPU con operaciones en `double` dentro de un lazo grande.

## Compilar (GCC)

```powershell
gcc -O2 -Wall -Wextra -std=c11 memory_stress.c -o memory_stress.exe
gcc -O2 -Wall -Wextra -std=c11 cpu_stress.c -o cpu_stress.exe -lm
```

## 1) Programa de consumo progresivo de memoria

```powershell
.\memory_stress.exe [MB_por_paso] [sleep_segundos] [max_pasos]
```

Ejemplo:

```powershell
.\memory_stress.exe 128 1 0
```

- `MB_por_paso`: memoria reservada en cada iteracion.
- `sleep_segundos`: pausa entre iteraciones.
- `max_pasos`: `0` para sin limite, o un numero para cortar antes.

En Windows, el programa informa la memoria fisica disponible aproximada en cada paso.

## 2) Programa de consumo de CPU

```powershell
.\cpu_stress.exe [iteraciones] [report_every]
```

Ejemplo:

```powershell
.\cpu_stress.exe 300000000 50000000
```

- `iteraciones`: total de iteraciones del lazo.
- `report_every`: frecuencia de mensajes de progreso.

## Monitorizacion recomendada

- Abrir Administrador de tareas (`Ctrl+Shift+Esc`) -> pestana `Rendimiento`.
- Para memoria: observar `Memoria` mientras corre `memory_stress.exe`.
- Para CPU: observar `CPU` y el proceso `cpu_stress.exe` en pestana `Detalles`.

## 3) Generar ensamblador con GCC

La opcion es `-S`:

```powershell
gcc -O2 -S cpu_stress.c -o cpu_stress.s
gcc -O2 -S memory_stress.c -o memory_stress.s
```

Esto genera los ficheros `.s` con el codigo ensamblador correspondiente.


- `entregable1_fork_info_mem_file.c`: puntos 1, 2 y 3.
- `entregable2_zombie_orfano_exec.c`: puntos 4, 5 y 6.
- `entregable3_sumatorio_fork.c`: punto 7 (opcion d, unica implementada).

### Compilar

En Linux/WSL/macOS:

```bash
gcc -O2 -Wall -Wextra -std=c11 entregable1_fork_info_mem_file.c -o entregable1
gcc -O2 -Wall -Wextra -std=c11 entregable2_zombie_orfano_exec.c -o entregable2
gcc -O2 -Wall -Wextra -std=c11 entregable3_sumatorio_fork.c -o entregable3 -lm
```

En este Windows (GCC UCRT sin POSIX completo) compilan con fallback y muestran mensaje de entorno requerido.

### Ejecutar

#### ENTREGABLE 1

```bash
./entregable1
- IDs y entorno con `getpid/getppid/getuid/geteuid/getgid/getenv`.
- Misma direccion virtual de variables global/local/dinamica en padre e hijo.
- Valores distintos tras modificar en el hijo (copy-on-write).
- Fichero abierto antes de `fork` compartido por padre e hijo.

#### ENTREGABLE 2

```bash
./entregable2 wait
- Estado zombie del hijo corto y `wait` del padre con `WEXITSTATUS`.
- Hijo largo huerfano cuando termina el padre.
- `execlp("ls", ...)` reemplaza imagen y el `printf` posterior no se ejecuta (si `exec` funciona).
- Diferencia entre terminar con `wait` y sin `wait` para el zombie.

#### ENTREGABLE 3

```bash
```

Comprueba:
	- Hijo-1 calcula pares.
	- Hijo-2 (nieto) nace despues y completa impares sobre la `sum` heredada.
	- Hijo-3 calcula referencia completa.
- Se imprime la diferencia absoluta entre resultado de hijo-2 y referencia.

- `entregable1_ctrlc_sigdfl.c`: punto 3, ENTREGABLE 1 (`SIGINT` personalizado hasta 8 veces y luego `SIG_DFL`).
- `practica3_4_waitpid_kill_hermanos.c`: punto 4 (`waitpid` sobre H1 y finalizacion de H1 por H2).
- `practica3_5_pause_sigusr1.c`: punto 5 (`pause` en hijo y desbloqueo con `SIGUSR1`).
- `entregable2_sigaction_bloqueo.c`: punto 6, ENTREGABLE 2 (`sigaction`, bloqueo pendiente de `SIGUSR1`, ignorar `SIGINT`).

### Compilar

En Linux/WSL/macOS:

```bash
gcc -O2 -Wall -Wextra -std=c11 practica3_1_infinito_sigkill.c -o p3_1
gcc -O2 -Wall -Wextra -std=c11 practica3_2_padre_hijo_signal.c -o p3_2
gcc -O2 -Wall -Wextra -std=c11 entregable1_ctrlc_sigdfl.c -o p3_ent1
gcc -O2 -Wall -Wextra -std=c11 practica3_4_waitpid_kill_hermanos.c -o p3_4
gcc -O2 -Wall -Wextra -std=c11 practica3_5_pause_sigusr1.c -o p3_5
gcc -O2 -Wall -Wextra -std=c11 entregable2_sigaction_bloqueo.c -o p3_ent2
```

En Windows, los programas muestran mensaje de entorno POSIX requerido.

### Ejecutar

```bash
./p3_1
# en otra consola:
kill -9 <pid>

./p3_2
./p3_ent1
./p3_4
./p3_5
Comprobaciones clave:
- En ENTREGABLE 1, los primeros 8 `Ctrl+C` no terminan el proceso; el 9º si lo termina.
- En punto 5, el hijo sale de `pause` al recibir `SIGUSR1` capturada.
- En ENTREGABLE 2, el padre procesa `SIGUSR2` antes de `SIGUSR1` al desbloquearla, y `SIGINT` de H2 se ignora.

- `practica4_1_variables_hilos.c`: punto 1 (acceso compartido a variables globales y por puntero).
- `practica4_2_tres_hilos_fork.c`: punto 2 (3 hilos, fork desde el tercero, comprobacion de hilos en el hijo).
- `practica4_3_exit_vs_pthread_exit.c`: punto 3 (efecto de `exit()` lanzado por un hilo).
- `entregable4_sumatorio_sqrt_hilos.c`: punto 4, ENTREGABLE (sumatorio de raices con reparto por modulo entre hilos).

### Compilar

En Linux/WSL/macOS (incluyendo `-lpthread`):

```bash
gcc -O2 -Wall -Wextra -std=c11 practica4_1_variables_hilos.c -o p4_1 -lpthread
gcc -O2 -Wall -Wextra -std=c11 practica4_2_tres_hilos_fork.c -o p4_2 -lpthread
gcc -O2 -Wall -Wextra -std=c11 practica4_3_exit_vs_pthread_exit.c -o p4_3 -lpthread
gcc -O2 -Wall -Wextra -std=c11 entregable4_sumatorio_sqrt_hilos.c -o p4_ent -lpthread -lm
```

### Ejecutar

```bash
./p4_1
./p4_2
./p4_3
./p4_ent              # enunciado: N = Nc * 1e8
- Punto 1: los hilos observan cambios en variable global y en local del main accedida por puntero.
- Punto 2: el hijo creado con `fork` desde un proceso multihilo aparece con un unico hilo en `ps -T`.
- Punto 3: cuando un hilo llama `exit()`, se termina todo el proceso (todos los hilos).
- ENTREGABLE: comparar suma paralela vs secuencial, tiempos y speedup.

### Archivos

- `practica5_mapa_memoria.c`: programa principal con los puntos 1..5.
- `practica5_cos_exec.c`: binario auxiliar para el `exec` del punto 4 (calculo de cosenos).

### Compilar (WSL/Linux)

```bash
gcc -O2 -Wall -Wextra -std=c11 practica5_mapa_memoria.c -o p5_main -lpthread -lm
gcc -O2 -Wall -Wextra -std=c11 practica5_cos_exec.c -o p5_cos_exec -lm
```

### Ejecutar

Ejecucion interactiva (recomendada):

```bash
./p5_main ./p5_cos_exec
```

El programa pide Enter entre etapas para inspeccionar `/proc/PID/maps`.

Ejecucion automatizada rapida (sin interaccion):

```bash
- Punto 2: direcciones de parametros/locales de `f1` y `f2` (zona de pila).
- Punto 3: cambios en mapa antes/despues de `malloc`/`free`.
- Punto 4: diferencias del mapa del hijo antes de `malloc`, despues de `malloc` y tras `exec`.
- Punto 5 (ENTREGABLE): direcciones de global compartida y locales del main/hilos, mas pilas por TID.

- `practica6_1_mmap_ro.c`: punto 1 (open + fstat + lectura + mmap RO/PRIVATE + munmap).
- `practica6_2_mmap_rw_modes.c`: puntos 2, 3, 4 y 5 (shared/private, ro/rw, msync, comparacion de mapas).
- `proyeccion1.c`: ENTREGABLE, proceso 1.
- `proyeccion2.c`: ENTREGABLE, proceso 2.

### Compilar (WSL/Linux)

```bash
gcc -O2 -Wall -Wextra -std=c11 practica6_1_mmap_ro.c -o p6_1
gcc -O2 -Wall -Wextra -std=c11 practica6_2_mmap_rw_modes.c -o p6_2
gcc -O2 -Wall -Wextra -std=c11 proyeccion1.c -o proyeccion1
gcc -O2 -Wall -Wextra -std=c11 proyeccion2.c -o proyeccion2
```

### Ejecucion

Preparar archivo base para pruebas:

```bash
printf 'abcdefghijk' > fichero.txt
```

Punto 1:

```bash
./p6_1 fichero.txt
```

Puntos 2-5 (comparar efectos):

```bash
./p6_2 fichero.txt shared rw XYZ
./p6_2 fichero.txt private rw QQQ
./p6_2 fichero.txt shared ro
```

ENTREGABLE (dos terminales):

Terminal 1:

```bash
./proyeccion1
```

Terminal 2:

```bash
./proyeccion2
```

Verificacion:

```bash
cat fichero.txt
cat fichero_tr.txt
```

Resultado esperado en ambos archivos tras sincronizacion: `0000001234abcd`.


### Archivo

- `archivos.c`: implementa los puntos 1..5 del enunciado.

### Compilar (WSL/Linux)

```bash
gcc -O2 -Wall -Wextra -std=c11 archivos.c -o archivos
```

### Preparar estructura de prueba

```bash
mkdir -p practica7/prueba/a_sub_dir
printf 'x' > practica7/prueba/a1
printf 'x' > practica7/prueba/e1
printf 'x' > practica7/prueba/A1
printf 'x' > practica7/prueba/b1
printf 'x' > practica7/prueba/c2
```

### Ejecutar

```bash
./archivos practica7/prueba 0666
- `fichero.txt` cambia permisos a `0760`.
- Se listan entradas del directorio de trabajo con `getcwd/opendir/readdir`.
- En `practica7/prueba` se muestran atributos solo para archivos regulares cuyo nombre empieza por vocal minuscula.
- Se cambian permisos de todos los archivos regulares de `practica7/prueba` al modo octal indicado por argumento.
