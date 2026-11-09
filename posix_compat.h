/*
 * Utilidades de diagnostico POSIX para Linux y macOS.
 * macOS no dispone de /proc ni de SYS_gettid: se utilizan APIs nativas.
 */
#ifndef PRACTICAS_POSIX_COMPAT_H
#define PRACTICAS_POSIX_COMPAT_H

#ifndef _WIN32
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_vm.h>
#include <mach/vm_region.h>
#include <sys/sysctl.h>
#else
#include <sys/syscall.h>
#endif

static inline long so_online_cpus(void)
{
#ifdef __APPLE__
    int count = 0;
    size_t length = sizeof(count);
    if (sysctlbyname("hw.logicalcpu", &count, &length, NULL, 0) == 0 && count > 0)
        return (long)count;
#endif
#ifdef _SC_NPROCESSORS_ONLN
    long count_posix = sysconf(_SC_NPROCESSORS_ONLN);
    if (count_posix > 0)
        return count_posix;
#endif
    return 1;
}

static inline unsigned long long so_thread_id(void)
{
#ifdef __APPLE__
    uint64_t tid = 0;
    if (pthread_threadid_np(NULL, &tid) == 0)
        return (unsigned long long)tid;
    return 0;
#else
    return (unsigned long long)syscall(SYS_gettid);
#endif
}

static inline void so_print_maps(const char *title)
{
    printf("\n========== %s (PID=%ld) ==========\n", title, (long)getpid());
#ifdef __APPLE__
    /* Enumerar las regiones del propio proceso sin depender de vmmap ni permisos de depuracion. */
    mach_vm_address_t address = 0;
    for (;;)
    {
        mach_vm_size_t size = 0;
        vm_region_basic_info_data_64_t info;
        mach_msg_type_number_t count = VM_REGION_BASIC_INFO_COUNT_64;
        mach_port_t object = MACH_PORT_NULL;
        kern_return_t kr = mach_vm_region(mach_task_self(), &address, &size,
                                          VM_REGION_BASIC_INFO_64,
                                          (vm_region_info_t)&info, &count, &object);
        if (kr != KERN_SUCCESS)
            break;
        if (object != MACH_PORT_NULL)
            mach_port_deallocate(mach_task_self(), object);

        printf("%016llx-%016llx %c%c%c %s\n",
               (unsigned long long)address,
               (unsigned long long)(address + size),
               (info.protection & VM_PROT_READ) ? 'r' : '-',
               (info.protection & VM_PROT_WRITE) ? 'w' : '-',
               (info.protection & VM_PROT_EXECUTE) ? 'x' : '-',
               (info.shared) ? "shared" : "private");

        if (size == 0 || address + size <= address)
            break;
        address += size;
    }
#else
    char path[64];
    snprintf(path, sizeof(path), "/proc/%ld/maps", (long)getpid());
    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror("fopen /proc maps");
        return;
    }
    char line[1024];
    while (fgets(line, sizeof(line), f))
        fputs(line, stdout);
    fclose(f);
#endif
    printf("========== fin maps ==========\n");
    fflush(stdout);
}

static inline void so_print_current_stack(void)
{
#ifdef __APPLE__
    pthread_t thread = pthread_self();
    void *high = pthread_get_stackaddr_np(thread);
    size_t size = pthread_get_stacksize_np(thread);
    printf("[STACK-MAP] TID=%llu rango-pila=%p-%p tamanio=%zu bytes\n",
           so_thread_id(), (void *)((char *)high - size), high, size);
#else
    unsigned long long tid = so_thread_id();
    char path[128];
    snprintf(path, sizeof(path), "/proc/%ld/task/%llu/maps", (long)getpid(), tid);
    FILE *f = fopen(path, "r");
    if (!f)
    {
        perror("fopen task maps");
        return;
    }
    printf("[STACK-MAP] TID=%llu archivo=%s\n", tid, path);
    char line[1024];
    while (fgets(line, sizeof(line), f))
    {
        if (strstr(line, "[stack") != NULL)
            fputs(line, stdout);
    }
    fclose(f);
#endif
}
#endif /* !_WIN32 */

#endif /* PRACTICAS_POSIX_COMPAT_H */
