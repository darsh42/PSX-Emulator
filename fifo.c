#include "fifo.h"

uint32_t fifo_length     ( struct fifo *fifo ) { assert(fifo); return fifo->length; }
uint32_t fifo_full       ( struct fifo *fifo ) { assert(fifo); return fifo->length == fifo->size; }
uint32_t fifo_empty      ( struct fifo *fifo ) { assert(fifo); return fifo->length == 0; }
uint32_t fifo_has_length ( struct fifo *fifo, int length ) { assert(fifo); return !(fifo_length(fifo) < length); }
uint32_t fifo_peek       ( struct fifo *fifo )
{
    assert(fifo);
    assert(!fifo_empty(fifo));
    return fifo->data[fifo->head];
}
uint32_t fifo_pop       ( struct fifo *fifo )
{
    assert(fifo);
    assert(!fifo_empty(fifo));

    uint32_t contents = fifo->data[fifo->head++];

    fifo->head %= fifo->size;
    fifo->length--;

    return contents;
}
void fifo_push          ( struct fifo *fifo, uint32_t data )
{
    assert(fifo);
    assert(!fifo_full(fifo));
    
    fifo->data[fifo->tail++] = data;

    fifo->tail %= fifo->size;
    fifo->length++;
}
uint32_t *fifo_push_ptr ( struct fifo *fifo )
{
    assert(fifo);
    assert(!fifo_full(fifo));

    uint32_t *ptr = &fifo->data[fifo->tail++];

    fifo->tail %= fifo->size;
    fifo->length++;
    return ptr;
}
void fifo_reset         ( struct fifo *fifo )     
{
    assert(fifo);

    fifo->head   = 0;
    fifo->tail   = 0;
    fifo->length = 0;
}
void fifo_create        ( struct fifo *fifo, uint32_t size )
{
    assert(fifo);
    assert(size);
    
    fifo->data = malloc(sizeof(uint32_t) * size);
    fifo->size = size;
}
void fifo_destroy       ( struct fifo *fifo )
{
    assert(fifo);
    assert(fifo->data);

    fifo_reset(fifo);
    fifo->size = 0;

    free(fifo->data);
}

static void fifo_test   ( void )
{
    struct fifo fifo;
    fifo_create(&fifo, 5);          // create fifo of size 5
    fifo_push  (&fifo, 1);          // push 1 onto it
    fifo_push  (&fifo, 2);          // push 2 onto it
    fifo_push  (&fifo, 3);          // push 3 onto it 
    fifo_push  (&fifo, 4);          // push 4 onto it 
    fifo_push  (&fifo, 5);          // push 5 onto it
    assert(fifo_peek(&fifo) == 1);  // peek the first element, should be 1
    assert(fifo_pop(&fifo) == 1);   // pop the first element, should be 1
    assert(fifo_peek(&fifo) == 2);  // peek the first element, should be 2
    assert(fifo_pop(&fifo) == 2);   // pop the first element, should be 2 
    fifo_destroy(&fifo);            // desroy the fifo
}
