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

#ifdef DEBUG
#include "stub.h"
#endif

uint32_t running = 1;

void usage( void )
{
    fprintf(stdout, "usage: psx -b bios -g game\n");
    fprintf(stdout, "   bios: path to bios\n");
    fprintf(stdout, "   game: path to game\n");
    exit(1);
}

int main( int argc , char **argv ) 
{
    // char *bios = "SCPH1001.BIN";
    char *bios = NULL;
    char *game = NULL;
    uint32_t debug = 0;

    char opt;

    while ((opt = getopt(argc, argv, "b:g:h")) != -1)
    {
        switch (opt)
        {
            case 'b': bios = optarg; break;
            case 'g': game = optarg; break;
            case 'h': 
                usage();
                break;
        }
    }

    if (!bios)
        usage();
    
    if (!game)
        usage();

    memory_load_bios( bios );

    pthread_t thread_cpu;
    pthread_t thread_gpu;
    pthread_t thread_dma;
    pthread_t thread_timers;
    // pthread_t thread_system;

#ifdef DEBUG
    pthread_t thread_gdbstub;

    assert(!pthread_create(&thread_gdbstub, NULL, task_gdb_stub, NULL));
#endif // DEBUG
    
    // assert(!pthread_create(&thread_system, NULL, task_system, NULL));

    assert(!pthread_create(&thread_dma,    NULL, task_dma,    NULL));
    assert(!pthread_create(&thread_gpu,    NULL, task_gpu,    NULL));
    assert(!pthread_create(&thread_cpu,    NULL, task_cpu,    NULL));
    assert(!pthread_create(&thread_timers, NULL, task_timers, NULL));

#ifdef DEBUG
    pthread_join(thread_gdbstub, NULL);
#endif
    
    /* signal to other threads to kill themselves */
    // pthread_join(thread_system, NULL);
    pthread_join(thread_cpu,    NULL);
    pthread_join(thread_gpu,    NULL);
    pthread_join(thread_dma,    NULL);
    pthread_join(thread_timers, NULL);

    return 0;
}
