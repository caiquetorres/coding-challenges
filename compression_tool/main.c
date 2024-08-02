#include <stdio.h>
#include <stdlib.h>

#define MAX_HEAP_SIZE 256
#define FILE_PATH_OPTION (1 << 0)
#define OUTPUT_FILE_PATH_OPTION (1 << 1)

typedef struct char_stream {
    FILE *file;
    char curr;
} char_stream;

char_stream *create_char_stream(char *);
char next_char(char_stream *);
char peek_char(char_stream *);
void destroy_char_stream(char_stream *);

void count_occurrences(long[256], char_stream *);

typedef struct huffman_node {
    char c;
    long frequency;
    struct huffman_node *left;
    struct huffman_node *right;
} huffman_node;

huffman_node *create_huffman_node(char, long, huffman_node *, huffman_node *);
huffman_node *create_huffman_tree(long [256]);
void print_huffman_node(huffman_node *);

typedef struct min_heap {
    int size;
    huffman_node *buffer[MAX_HEAP_SIZE];
} min_heap;

min_heap *create_heap();
void heap_push(min_heap *, huffman_node *);
huffman_node *heap_peek(min_heap *);
huffman_node *heap_pop(min_heap *);
int heap_size(min_heap *);
char is_heap_empty(min_heap *);
void _heap_heapify(min_heap *, int);
int _heap_parent(int);
int _heap_left(int);
int _heap_right(int);
void _heap_swap(huffman_node **, huffman_node **);

void print_binary(unsigned char);

int main() {
    char_stream *stream = create_char_stream("test/file.txt");
    long occ[256];
    count_occurrences(occ, stream);
    huffman_node *root = create_huffman_tree(occ);
    return 0;
}

char_stream *create_char_stream(char *file_path) {
    char_stream *stream = (char_stream *)malloc(sizeof(char_stream));
    FILE *file;
    file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "ccct: file %s not found.", file_path);
        exit(0);
    }
    stream->file = file;
    stream->curr = fgetc(file);
    return stream;
}

char peek_char(char_stream *stream) {
    return stream->curr;
}

char next_char(char_stream *stream) {
    char curr = stream->curr;
    stream->curr = fgetc(stream->file);
    return curr;
}

void destroy_char_stream(char_stream *stream) {
    fclose(stream->file);
}

void count_occurrences(long occ[256], char_stream *stream) {
    for (int i = 0; i < 256; i++) {
        *(occ + i) = 0;
    }
    while (peek_char(stream) != EOF) {
        char c = next_char(stream);
        *(occ + c) += 1;
    }
}

min_heap *create_heap() {
    min_heap *heap = NULL;
    heap = (min_heap *)malloc(4 * sizeof(min_heap));
    heap->size = 0;
    return heap;
}

void heap_push(min_heap *heap, huffman_node *node) {
    if (heap_size(heap) + 1 > MAX_HEAP_SIZE) {
        fprintf(stderr, "ccct: heap overflow error.\n");
        exit(0);
    }
    heap->size++;
    int i = heap->size - 1;
    *(heap->buffer + i) = node;
    int freq = (*(heap->buffer + i))->frequency;
    while (i > 0 && (*(heap->buffer + _heap_parent(i)))->frequency > (*(heap->buffer + i))->frequency) {
        _heap_swap(heap->buffer + i, heap->buffer + _heap_parent(i));
        i = _heap_parent(i);
    }
}

huffman_node *heap_peek(min_heap *heap) {
    if (heap->size == 0) {
        return NULL;
    }
    return *heap->buffer;
}

huffman_node *heap_pop(min_heap *heap) {
    if (is_heap_empty(heap)) {
        fprintf(stderr, "ccct: heap underflow error.\n");
        exit(0);
    }
    if (heap_size(heap) == 1) {
        heap->size--;
        return *heap->buffer;
    }
    huffman_node *root = *(heap->buffer);
    *(heap->buffer) = *(heap->buffer + heap->size - 1);
    heap->size--;
    _heap_heapify(heap, 0);
    return root;
}

int heap_size(min_heap *heap) {
    return heap->size;
}

char is_heap_empty(min_heap *heap) {
    return heap->size == 0;
}

void _heap_heapify(min_heap *heap, int i) {
    int left = _heap_left(i);
    int right = _heap_right(i);
    int smallest = i;
    if (left < heap->size && (*(heap->buffer + left))->frequency < (*(heap->buffer + i))->frequency) {
        smallest = left;
    }
    if (right < heap->size && (*(heap->buffer + right))->frequency < (*(heap->buffer + smallest))->frequency) {
        smallest = right;
    }
    if (smallest != i) {
        _heap_swap(heap->buffer + i, heap->buffer + smallest);
        _heap_heapify(heap, smallest);
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

void _heap_swap(huffman_node **x, huffman_node **y) {
    huffman_node *temp = *x;
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
}

huffman_node *create_huffman_node(char c, long freq, huffman_node *left, huffman_node *right) {
    huffman_node *node = NULL;
    node = (huffman_node *)malloc(sizeof(huffman_node));
    node->c = c;
    node->frequency = freq;
    node->left = left;
    node->right = right;
    return node;
}

huffman_node *create_huffman_tree(long occurrences[256]) {
   min_heap *heap = create_heap();
    for (int c = 0; c < 256; c++) {
        if (occurrences[c] != 0) {
            huffman_node *node = create_huffman_node(c, occurrences[c], NULL, NULL);
            heap_push(heap, node);
        }
    }
    while (heap_size(heap) > 1) {
        huffman_node *left = heap_pop(heap);
        huffman_node *right = heap_pop(heap);
        long new_freq = left->frequency + right->frequency;
        huffman_node *new_node = create_huffman_node(' ', new_freq, left, right);
        heap_push(heap, new_node);
    }
    return heap_peek(heap);
}

void print_huffman_node(huffman_node *node) {
    printf("%c, %ld, %p, %p\n", node->c, node->frequency, node->left, node->right);
}
