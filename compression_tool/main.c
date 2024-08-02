#include <stdio.h>
#include <stdlib.h>

#define MAX_HEAP_SIZE 256

typedef struct heap_node {
    char value;
    int priority;
} heap_node;

typedef struct min_heap {
    int size;
    heap_node buffer[MAX_HEAP_SIZE];
} min_heap;

min_heap *create_heap();
void heap_push(min_heap *, char, int);
heap_node *heap_peek(min_heap *);
heap_node heap_pop(min_heap *);
void _heap_heapify(min_heap *, int);
void _heap_decrease(min_heap *, int, char, int);
int _heap_parent(int);
int _heap_left(int);
int _heap_right(int);
void _heap_swap(heap_node *, heap_node *);

void print_binary(unsigned char);

int main() {
    min_heap *heap = create_heap();
    heap_push(heap, 'c', 10);
    heap_push(heap, 'u', 1);
    heap_push(heap, 'a', 9);
    heap_push(heap, 'i', 12);
    heap_push(heap, 'q', 11);
    while (heap->size > 0) {
        heap_node node = heap_pop(heap);
        printf("%c, %d\n", node.value, node.priority);
    }
    return 0;
}

min_heap *create_heap() {
    min_heap *heap = NULL;
    heap = (min_heap *)malloc(4 * sizeof(min_heap));
    heap->size = 0;
    return heap;
}

void heap_push(min_heap *heap, char value, int priority) {
    if (heap->size + 1 > MAX_HEAP_SIZE) {
        fprintf(stderr, "ccct: heap overflow error.");
        exit(0);
    }
    heap_node node = { value, priority };
    heap->size++;
    int i = heap->size - 1;
    *(heap->buffer + i) = node;
    while (i > 0 && (heap->buffer + _heap_parent(i))->priority > (heap->buffer + i)->priority) {
        _heap_swap(heap->buffer + i, heap->buffer + _heap_parent(i));
        i = _heap_parent(i);
    }
}

heap_node *heap_peek(min_heap *heap) {
    if (heap->size == 0) {
        return NULL;
    }
    return heap->buffer;
}

heap_node heap_pop(min_heap *heap) {
    if (heap->size == 0) {
        fprintf(stderr, "ccct: heap underflow error.");
        exit(0);
    }
    if (heap->size == 1) {
        heap->size--;
        return *heap->buffer;
    }
    heap_node root = *(heap->buffer);
    *(heap->buffer) = *(heap->buffer + heap->size - 1);
    heap->size--;
    _heap_heapify(heap, 0);
    return root;
}

void _heap_heapify(min_heap *heap, int i) {
    int left = _heap_left(i);
    int right = _heap_right(i);
    int smallest = i;
    if (left < heap->size && (heap->buffer + left)->priority < (heap->buffer + i)->priority) {
        smallest = left;
    }
    if (right < heap->size && (heap->buffer + right)->priority < (heap->buffer + smallest)->priority) {
        smallest = right;
    }
    if (smallest != i) {
        _heap_swap(heap->buffer + i, heap->buffer + smallest);
        _heap_heapify(heap, smallest);
    }
}

void _heap_decrease(min_heap *heap, int i, char value, int priority) {
    heap_node node = { value, priority };
    *(heap->buffer + i) = node;
    while (i != 0 && (heap->buffer + _heap_parent(i))->priority > (heap->buffer + i)->priority) {
        _heap_swap(heap->buffer + i, heap->buffer + _heap_parent(i));
        i = _heap_parent(i);
    }
}

int _heap_parent(int i) {
    return (i - 1) / 2;
}

int _heap_left(int i) {
    return 2 * i + 1;
}

int _heap_right(int i) {
    return 2 * i + 2;
}

void _heap_swap(heap_node *x, heap_node *y) {
    heap_node temp = *x;
    *x = *y;
    *y = temp;
}

void print_binary(unsigned char c) {
    for (int i = 7; i >= 0; i--) {
        if ((c & (1 << i)) != 0) {
            printf("1");
        } else {
            printf("0");
        }
    }
    printf("\n");
}
