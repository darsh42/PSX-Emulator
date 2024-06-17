#include "datastructures.h"

struct FIFO {
    int *array, size, len;
    
    int head, tail;
};

bool fifo_full(fifo_t fifo) {
    return (fifo.len == fifo.size);
}

bool fifo_empty(fifo_t fifo) {
    return (fifo.len == 0);
}

void fifo_create(fifo_t *fifo, int size) {
    fifo->array = malloc(sizeof(fifo->array) * size);
    fifo->size = size;
    fifo->head = 0;
    fifo->tail = 0;
    fifo->len  = 0;
}

void fifo_destroy(fifo_t *fifo) {
    free(fifo->array);
}

int fifo_push(fifo_t *fifo, int elem) {
    if (fifo_full(*fifo)) 
        return -1;

    fifo->array[fifo->tail] = elem;

    fifo->tail++;
    fifo->tail %= fifo->size;

    fifo->len++;

    return 0;
}

int fifo_peek(fifo_t fifo) {
    return (fifo_empty(fifo)) ? -1: fifo.array[fifo.head];
}

int fifo_pop(fifo_t *fifo) {
    if (fifo_empty(*fifo)) 
        return -1;
    
    int value = fifo->array[fifo->head];

    fifo->head++;
    fifo->head %= fifo->size;

    fifo->len--;
    
    return value;
}

void fifo_print(fifo_t fifo) {
    for (int i = fifo.head, c = fifo.len; c > 0; c--, i++, i %= fifo.size)
        printf("[FIFO] index = %d, value = %d\n", i, fifo.array[i]);
}
