# Prácticas de Sistemas Operativos (C / POSIX)

Ejercicios de procesos, señales, hilos, memoria, proyecciones de archivos y sistema de archivos. Se pueden **compilar y ejecutar en Linux y macOS** (tanto Mac Intel como Apple Silicon) utilizando el código fuente C.

> ⚠️ Algunos ejercicios crean procesos zombis, envían señales o provocan carga de CPU y memoria. Ejecútelos en una terminal de usuario, en un directorio de pruebas, y **no utilice los modos ilimitados de estrés** sin supervisión.

## Requisitos

- **macOS:** instalar las Command Line Tools de Xcode si todavía no están disponibles: `xcode-select --install`. Incluyen `clang`, `make` y el SDK de macOS.
- **Linux:** `gcc` o `clang`, `make`, `python3` y bibliotecas estándar POSIX/pthreads.
- No se necesitan Homebrew, GNU `ps`, `/proc` ni librerías de terceros en macOS.

Desde la raíz del repositorio:

```bash
make                 # compila todos los .c en build/
make test            # ejecuta pruebas rápidas y acotadas
make clean
```

La configuración utiliza C11, `-Wall -Wextra`, soporte POSIX y enlaces con `-pthread -lm`. En macOS, `cc` es Apple Clang. Los binarios se colocan en `build/` para no confundirse con los fuentes.

Para compilar manualmente un ejercicio:

```bash
cc -O2 -Wall -Wextra -std=c11 -D_POSIX_C_SOURCE=200809L -D_DARWIN_C_SOURCE=1 \
  practica6_1_mmap_ro.c -o p6_1 -pthread -lm
```

En Linux se puede reemplazar `-D_DARWIN_C_SOURCE=1` por `-D_DEFAULT_SOURCE=1`. Es preferible usar `make`, que detecta ambos sistemas.

## Ejercicios

| Tema | Código fuente | Ejemplo |
|---|---|---|
| Carga de CPU y memoria | `cpu_stress.c`, `memory_stress.c` | `./build/memory_stress 16 1 3` |
| Procesos, `fork` y ficheros compartidos | `entregable1_fork_info_mem_file.c` | `./build/entregable1_fork_info_mem_file` |
| Zombis, huérfanos y `exec` | `entregable2_zombie_orfano_exec.c` | `./build/entregable2_zombie_orfano_exec wait` |
| Sumatorio por procesos | `entregable3_sumatorio_fork.c` | `./build/entregable3_sumatorio_fork` |
| Señales entre procesos | `practica3_1_infinito_sigkill.c`, `practica3_2_padre_hijo_signal.c`, `practica3_4_waitpid_kill_hermanos.c`, `practica3_5_pause_sigusr1.c` | `./build/practica3_5_pause_sigusr1` |
| Bloqueo de señales y Ctrl+C | `entregable1_ctrlc_sigdfl.c`, `entregable2_sigaction_bloqueo.c` | `./build/entregable1_ctrlc_sigdfl` |
| Optativas: señales, zombis y sumatorio | `opt2_1_hijo1_senhala_hijo2_pipe.c`, `opt2_2_senales_sin_parentesco.c`, `opt2_3_zombies_huerfanos.c`, `opt2_4_sumatorio_sqrt_procesos.c` | `./build/opt2_4_sumatorio_sqrt_procesos 10000` |
| Hilos y `fork` | `practica4_1_variables_hilos.c`, `practica4_2_tres_hilos_fork.c`, `practica4_3_exit_vs_pthread_exit.c` | `./build/practica4_2_tres_hilos_fork` |
| Sumatorio paralelo con pthreads | `entregable4_sumatorio_sqrt_hilos.c` | `./build/entregable4_sumatorio_sqrt_hilos 10000` |
| Mapas de memoria y cambio de imagen | `practica5_mapa_memoria.c`, `practica5_cos_exec.c` | `./build/practica5_mapa_memoria ./build/practica5_cos_exec` |
| `mmap` privado/compartido | `practica6_1_mmap_ro.c`, `practica6_2_mmap_rw_modes.c` | Véase más abajo |
| Dos procesos con proyecciones | `proyeccion1.c`, `proyeccion2.c` | Ejecutar en terminales distintas |
| Archivos y permisos | `archivos.c` | `./build/archivos practica7/prueba 0666` |

### Ejemplos específicos

**Señales entre procesos independientes** (dos terminales):

```bash
./build/opt2_2_senales_sin_parentesco ejemplo A
./build/opt2_2_senales_sin_parentesco ejemplo B
```

Los PID se intercambian por archivos temporales bajo `/tmp`; use un nombre de canal nuevo por ejecución y asegúrese de que no existan instancias antiguas del mismo canal.

**Proyecciones de memoria**:

```bash
printf 'abcdefghijk' > fichero.txt
./build/practica6_1_mmap_ro fichero.txt
./build/practica6_2_mmap_rw_modes fichero.txt private rw QQQ
./build/practica6_2_mmap_rw_modes fichero.txt shared rw XYZ
./build/practica6_2_mmap_rw_modes fichero.txt shared ro
```

Para el entregable, ejecute `./build/proyeccion1` y, mientras está esperando, `./build/proyeccion2`. El contenido final esperado de `fichero.txt` y `fichero_tr.txt` es `0000001234abcd`.

**Sistema de archivos** (en un directorio de trabajo donde se permita crear y cambiar archivos):

```bash
mkdir -p practica7/prueba
printf 'x' > practica7/prueba/a1
./build/archivos practica7/prueba 0666
```

El programa `archivos` crea los ficheros `fichero.txt` y `fichero_2.txt` en el directorio actual y modifica permisos de los ficheros regulares de la ruta indicada; use una copia de prueba.

**Ctrl+C**: en `entregable1_ctrlc_sigdfl`, las ocho primeras señales `SIGINT` se contabilizan y se restaura el comportamiento predeterminado para la siguiente.

## Diferencias relevantes de macOS

- macOS **no tiene `/proc/PID/maps`**. `posix_compat.h` enumera las regiones de memoria del propio proceso mediante la API Mach y obtiene las direcciones de pila con `pthread_get_stackaddr_np`/`pthread_get_stacksize_np`. Linux sigue utilizando `/proc`.
- Para el identificador de hilo se utiliza `pthread_threadid_np` en macOS y `gettid` mediante `syscall` en Linux.
- El número de procesadores lógicos se obtiene mediante `sysctlbyname("hw.logicalcpu")` en macOS y `sysconf` donde está disponible.
- `ps -T` es una opción de Linux; la práctica de hilos usa `ps -M` en macOS y realiza la inspección desde el proceso padre.
- Los procesos huérfanos no dependen de `systemd` en macOS: la adopción depende del sistema y de `launchd`.
- Los archivos `cpu_stress.s` y `memory_stress.s` son **salidas ensamblador conservadas del entorno original**. No son ensamblador universal para x86_64 y arm64: genere la versión adecuada para su Mac usando `cc -O2 -S cpu_stress.c -o cpu_stress_mac.s`, y análogamente con `memory_stress.c`.

## Diagnóstico y pruebas

- **macOS:** Monitor de Actividad (CPU y Memoria), `top -pid <PID>`, `vm_stat` y `ps -M -p <PID>`.
- **Linux:** `top`, `ps -T -p <PID>` y `cat /proc/<PID>/maps`.
- **CI:** `.github/workflows/posix-ci.yml` compila todos los `.c` y ejecuta `tests/smoke.py` en `ubuntu-latest` y `macos-latest`. Los casos interactivos se alimentan automáticamente; los infinitos se detienen mediante señales; las pruebas de escritura usan directorios temporales.

Las prácticas ilustran aspectos concretos de POSIX; no todas las características internas del kernel (zombis, direcciones de memoria, numeración de hilos o formato exacto de `ps`) tienen representaciones idénticas en Linux y macOS.

**Windows nativo:** los ejercicios que requieren `fork`, señales POSIX o `mmap` permanecen restringidos a sistemas POSIX; en Windows pueden ejecutarse desde WSL.
