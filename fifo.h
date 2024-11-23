#ifndef FIFO_H_INCLUDED
#define FIFO_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

struct fifo {

    uint32_t *data;  // contents of fifo
    uint32_t size;   // max size of fifo

    uint32_t head;   // first element in fifo
    uint32_t tail;   // last element  in fifo
    uint32_t length; // current length of fifo

};

void      fifo_create     ( struct fifo *fifo, uint32_t size );
void      fifo_destroy    ( struct fifo *fifo );
uint32_t  fifo_length     ( struct fifo *fifo );
uint32_t  fifo_has_length ( struct fifo *fifo, int length );
uint32_t  fifo_full       ( struct fifo *fifo );
uint32_t  fifo_empty      ( struct fifo *fifo );
void      fifo_reset      ( struct fifo *fifo );
void      fifo_push       ( struct fifo *fifo, uint32_t data );
uint32_t *fifo_push_ptr   ( struct fifo *fifo );
uint32_t  fifo_peek       ( struct fifo *fifo );
uint32_t  fifo_pop        ( struct fifo *fifo );

#endif // FIFO_H_INCLUDED
