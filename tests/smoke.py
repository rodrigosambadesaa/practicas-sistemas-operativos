#!/usr/bin/env python3
"""Pruebas de ejecucion rapidas, aisladas y con timeout para Linux/macOS."""
import os
import pathlib
import signal
import subprocess
import tempfile
import time

ROOT = pathlib.Path(__file__).resolve().parents[1]
BIN = ROOT / "build"


def check(condition, message):
    if not condition:
        raise AssertionError(message)


def run(name, *args, cwd, timeout=20, expected=0, stdin=None):
    command = [str(BIN / name), *map(str, args)]
    result = subprocess.run(command, cwd=cwd, input=stdin, text=True,
                            stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                            timeout=timeout, check=False)
    check(result.returncode == expected,
          f"{name}: exit={result.returncode} (esperado={expected})\n{result.stdout[-5000:]}")
    print(f"[OK] {name}")
    return result.stdout


def run_pair(cwd, args_a, args_b, expected_a=0, expected_b=0, timeout=15):
    a = subprocess.Popen([str(BIN / args_a[0]), *args_a[1:]], cwd=cwd,
                         stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
    b = None
    try:
        time.sleep(0.25)
        b = subprocess.Popen([str(BIN / args_b[0]), *args_b[1:]], cwd=cwd,
                             stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        stdout_a, _ = a.communicate(timeout=timeout)
        stdout_b, _ = b.communicate(timeout=timeout)
        check(a.returncode == expected_a, f"{args_a}: {stdout_a}")
        check(b.returncode == expected_b, f"{args_b}: {stdout_b}")
        print(f"[OK] {args_a[0]} + {args_b[0]}")
        return stdout_a, stdout_b
    finally:
        for proc in (a, b):
            if proc is not None and proc.poll() is None:
                proc.kill()
                proc.communicate()


def main():
    with tempfile.TemporaryDirectory(prefix="so-smoke-") as tmp:
        cwd = pathlib.Path(tmp)
        run("cpu_stress", 10000, 1000, cwd=cwd)
        run("memory_stress", 1, 0, 2, cwd=cwd)
        run("entregable1_fork_info_mem_file", cwd=cwd)
        check((cwd / "entregable1_shared.log").exists(), "No se creo el log del fork")
        output = run("entregable3_sumatorio_fork", cwd=cwd)
        check("opcion d valida" in output, "Sumatorio de procesos no valido")
        run("entregable4_sumatorio_sqrt_hilos", 10000, cwd=cwd)
        run("opt2_4_sumatorio_sqrt_procesos", 10000, cwd=cwd)
        run("practica3_2_padre_hijo_signal", cwd=cwd)
        run("practica3_4_waitpid_kill_hermanos", cwd=cwd)
        run("practica3_5_pause_sigusr1", cwd=cwd)
        run("entregable2_sigaction_bloqueo", cwd=cwd)
        run("opt2_1_hijo1_senhala_hijo2_pipe", cwd=cwd)
        canal = f"so-test-{os.getpid()}"
        run_pair(cwd, ["opt2_2_senales_sin_parentesco", canal, "A"],
                 ["opt2_2_senales_sin_parentesco", canal, "B"])
        run("opt2_3_zombies_huerfanos", 1, 1, cwd=cwd, timeout=20)
        run("entregable2_zombie_orfano_exec", "wait", cwd=cwd, timeout=12)

        run("practica4_1_variables_hilos", cwd=cwd)
        run("practica4_2_tres_hilos_fork", cwd=cwd)
        run("practica4_3_exit_vs_pthread_exit", cwd=cwd, expected=7)
        run("practica5_cos_exec", cwd=cwd)
        out = run("practica5_mapa_memoria", str(BIN / "practica5_cos_exec"),
                  cwd=cwd, stdin="\n" * 8)
        check("Practica 5 completada" in out, "Practica 5 incompleta")

        p = cwd / "dato.txt"
        p.write_text("abcdefghijk", encoding="utf-8")
        run("practica6_1_mmap_ro", str(p), cwd=cwd)
        run("practica6_2_mmap_rw_modes", str(p), "private", "rw", "QQQ", cwd=cwd)
        check(p.read_text() == "abcdefghijk", "MAP_PRIVATE ha modificado el archivo")
        run("practica6_2_mmap_rw_modes", str(p), "shared", "rw", "XYZ", cwd=cwd)
        check(p.read_text().startswith("XYZ"), "MAP_SHARED no ha persistido")
        run("practica6_2_mmap_rw_modes", str(p), "shared", "ro", cwd=cwd)

        # El primer proceso inicia la proyeccion; el segundo actualiza su mmap.
        run_pair(cwd, ["proyeccion1"], ["proyeccion2"], timeout=15)
        check((cwd / "fichero.txt").read_text() == "0000001234abcd",
              "Proyeccion 2: contenido inesperado")
        check((cwd / "fichero_tr.txt").read_text() == "0000001234abcd",
              "Proyeccion 1: copia inesperada")

        directory = cwd / "prueba"
        directory.mkdir()
        (directory / "a1").write_text("x")
        (directory / "b1").write_text("x")
        output = run("archivos", str(directory), "0644", cwd=cwd)
        check("Suma total de espacio" in output, "No se listaron atributos")

        # Programa deliberadamente infinito: comprobar SIGKILL sin dejar procesos vivos.
        p_inf = subprocess.Popen([str(BIN / "practica3_1_infinito_sigkill")],
                                 cwd=cwd, stdout=subprocess.DEVNULL)
        try:
            time.sleep(0.15)
            p_inf.kill()
            check(p_inf.wait(timeout=5) == -signal.SIGKILL, "SIGKILL inesperada")
            print("[OK] practica3_1_infinito_sigkill")
        finally:
            if p_inf.poll() is None:
                p_inf.kill()
                p_inf.wait()

        # Sigwait recibe 8 interrupciones y la novena aplica SIG_DFL.
        p_int = subprocess.Popen([str(BIN / "entregable1_ctrlc_sigdfl")],
                                 cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.STDOUT,
                                 text=True)
        try:
            time.sleep(0.25)
            for _ in range(9):
                p_int.send_signal(signal.SIGINT)
                time.sleep(0.1)
            stdout, _ = p_int.communicate(timeout=5)
            check("capturado (8/8)" in stdout, stdout)
            check(p_int.returncode == -signal.SIGINT, f"SIGINT exit={p_int.returncode}")
            print("[OK] entregable1_ctrlc_sigdfl")
        finally:
            if p_int.poll() is None:
                p_int.kill()
                p_int.communicate()
    print("Todos los smoke tests multiplataforma han terminado correctamente.")


if __name__ == "__main__":
    main()
