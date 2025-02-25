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

#define TRACE_H_IMPLEMENTATION
#include "trace.h"

#ifdef DEBUG
#include "stub.h"
#endif

uint32_t running = 1;

void *task_core( void * )
{
    printf("CORE: %ld\n", pthread_self());

	init_cpu();
	init_gpu();
	init_dma();
	init_timers();
	init_interrupts();
		
    uint32_t ticks_till_cpu = 0;
    uint32_t ticks_till_gpu = 0;

    while (running)
    {
        /* clock */
        task_timers();

        ticks_till_cpu++;
        ticks_till_gpu++;
        
        if (ticks_till_cpu == 11) { task_cpu(); ticks_till_cpu = 0; }
        if (ticks_till_gpu ==  7) { task_gpu(); ticks_till_gpu = 0; }

        task_dma();
    }

    return NULL;
}


void *task_debug( void * )
{
    printf("DEBUGGER: %ld\n", pthread_self());

    return NULL;
}

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
	
    pthread_t thread_core;
    pthread_t thread_debug;
    pthread_t thread_system;

    assert(!pthread_create(&thread_system, NULL, task_system, NULL));
    assert(!pthread_create(&thread_debug, NULL, task_debug, NULL));
    assert(!pthread_create(&thread_core, NULL, task_core, NULL));

    pthread_join(thread_core, NULL);
    pthread_join(thread_debug, NULL);
    pthread_join(thread_system, NULL);

    return 0;
}
