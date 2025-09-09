#include <pthread.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "cpu.h"
#include "gpu.h"
#include "dma.h"
#include "timer.h"
#include "memory.h"
#include "system.h"
#include "interrupts.h"

#define TRACE_H_IMPLEMENTATION
#include "trace.h"

#include "stub.h"

uint32_t running = 1;

struct task_core_args
{
    const char *bios;
    const char *exe;
    const char *tty;
};

void *task_core( void *_args )
{
    printf("CORE: %ld\n", pthread_self());

    struct task_core_args *args = (struct task_core_args *) _args;

    init_tracer();

#ifdef GDBSTUB
    init_gdb_stub();
#endif

    init_cpu(args->bios,
             args->exe);
    init_gpu();
    init_dma();
    init_timers();
    init_interrupts();

    uint32_t ticks_till_cpu = 0;
    uint32_t ticks_till_gpu = 0;
    uint32_t ticks_till_spu = 0;

    while (running)
    {
#ifdef GDBSTUB
        /* check if user interacting with stub */
        while (paused_gdb_stub())
            ;
#endif // GDBSTUB

        /* clock */
        task_timers();

        ticks_till_cpu++;
        ticks_till_gpu++;

        /* devices synched to cpu clock */
        if (ticks_till_cpu == 11)
        {
            task_cpu();
            task_dma();

            if (ticks_till_spu == 768)
                task_spu();

            ticks_till_cpu = 0;
            ticks_till_spu++;
        }


        /* devices synched to gpu clock */
        if (ticks_till_gpu ==  7)
        {
            ticks_till_gpu = 0;

            task_gpu();
        }
        // task_cpu();
        // task_dma();
        // task_spu();
        // task_gpu();
    }

    return NULL;
}

#ifdef GDBSTUB
void *task_debug( void * )
{
    printf("DEBUGGER: %ld\n", pthread_self());

    init_gdb_stub();

    while (running)
        task_gdb_stub();

    kill_gdb_stub();

    return NULL;
}
#endif // GDBSTUB

int main( int argc , char **argv )
{
    char *bios = NULL;
    char *exe  = NULL;
    char *tty  = NULL;
    char *game = NULL;

    char opt;

    while ((opt = getopt(argc, argv, "b:e:g:h")) != -1)
    {
        switch (opt)
        {
            case 'b': bios = optarg; break;
            case 'g': game = optarg; break;
            case 'e': exe  = optarg; break;
            case 'h':
            default:
                goto usage;
                break;
        }
    }

    /* set initial trace state */
    // trace_set_profile(TRACE_GPU_EN);


    if (!bios || !game)
        goto usage;


    struct task_core_args core_args = {
        .bios = bios,
        .exe  = exe,
        .tty  = tty
    };

    pthread_t thread_core;
    pthread_t thread_system;

#ifdef GDBSTUB
    pthread_t thread_debug;
    assert(!pthread_create(&thread_debug, NULL, task_debug, NULL));
#endif // GDBSTUB

    assert(!pthread_create(&thread_system,
                NULL, task_system, NULL));
    wait_system_ready();
    assert(!pthread_create(&thread_core,
                NULL, task_core, (void *) &core_args));

#ifdef GDBSTUB
    pthread_join(thread_debug, NULL);
#endif // GDBSTUB
    pthread_join(thread_core, NULL);
    pthread_join(thread_system, NULL);

    return 0;

usage:
    fprintf(stderr, "usage: %s BIOS.bin GAME.bin\n", *argv);
    fprintf(stderr, "   BIOS.bin: path to bios\n");
    fprintf(stderr, "   GAME.bin: path to game\n");

    return 1;
}
