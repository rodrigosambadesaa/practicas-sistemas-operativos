#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
static void sleep_seconds(unsigned int seconds)
{
    Sleep(seconds * 1000U);
}

static unsigned long long available_physical_mb(void)
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    if (GlobalMemoryStatusEx(&statex) == 0)
    {
        return 0;
    }
    return (unsigned long long)(statex.ullAvailPhys / (1024ULL * 1024ULL));
}
#else
#include <unistd.h>
static void sleep_seconds(unsigned int seconds)
{
    sleep(seconds);
}

static unsigned long long available_physical_mb(void)
{
    return 0;
}
#endif

int main(int argc, char *argv[])
{
    size_t mb_per_step = 64;
    unsigned int sleep_s = 1;
    size_t max_steps = 0;

    if (argc > 1)
    {
        mb_per_step = (size_t)strtoull(argv[1], NULL, 10);
    }
    if (argc > 2)
    {
        sleep_s = (unsigned int)strtoul(argv[2], NULL, 10);
    }
    if (argc > 3)
    {
        max_steps = (size_t)strtoull(argv[3], NULL, 10);
    }

    if (mb_per_step == 0)
    {
        fprintf(stderr, "Error: MB por paso debe ser > 0.\n");
        return 1;
    }

    const size_t bytes_per_step = mb_per_step * 1024ULL * 1024ULL;

    void **blocks = NULL;
    size_t block_count = 0;
    size_t reserved_mb = 0;

    printf("Iniciando consumo progresivo de memoria...\n");
    printf("Paso: %zu MB | Sleep: %u s | Max pasos: %s\n",
           mb_per_step, sleep_s, max_steps ? "definido" : "sin limite");

    while (max_steps == 0 || block_count < max_steps)
    {
        void **tmp = (void **)realloc(blocks, (block_count + 1) * sizeof(void *));
        if (tmp == NULL)
        {
            fprintf(stderr, "realloc fallo tras reservar %zu MB totales.\n", reserved_mb);
            break;
        }
        blocks = tmp;

        blocks[block_count] = malloc(bytes_per_step);
        if (blocks[block_count] == NULL)
        {
            fprintf(stderr, "malloc fallo en el paso %zu. Reservado: %zu MB\n", block_count + 1, reserved_mb);
            break;
        }

        memset(blocks[block_count], 0xA5, bytes_per_step);

        block_count++;
        reserved_mb += mb_per_step;

        unsigned long long avail_mb = available_physical_mb();
        if (avail_mb > 0)
        {
            printf("Paso %zu: reservado total %zu MB | fisica disponible aprox %llu MB\n",
                   block_count, reserved_mb, avail_mb);
        }
        else
        {
            printf("Paso %zu: reservado total %zu MB\n", block_count, reserved_mb);
        }

        fflush(stdout);
        sleep_seconds(sleep_s);
    }

    printf("Liberando memoria...\n");
    for (size_t i = 0; i < block_count; i++)
    {
        free(blocks[i]);
    }
    free(blocks);
    printf("Fin.\n");

    return 0;
}
