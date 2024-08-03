#include <stdio.h>
#include <stdlib.h>

#define MAX_HEAP_SIZE 256
#define FILE_PATH_OPTION (1 << 0)
#define OUTPUT_FILE_PATH_OPTION (1 << 1)

typedef struct file_stream_reader {
    FILE *file;
    char curr;
    char cur_bit;
} file_stream_reader;

file_stream_reader *create_file_stream_reader(char *);
char next_byte(file_stream_reader *);
char peek_byte(file_stream_reader *);
char next_bit(file_stream_reader *);
char peek_bit(file_stream_reader *);
void reset_file_stream_reader(file_stream_reader *);
void destroy_file_stream_reader(file_stream_reader *);

void count_occurrences(long[256], file_stream_reader *);

typedef struct huffman_node {
    unsigned char value;
    long frequency;
    struct huffman_node *left;
    struct huffman_node *right;
} huffman_node;

huffman_node *create_huffman_node(unsigned char, long, huffman_node *, huffman_node *);
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

typedef struct linked_list_node {
    unsigned char value;
    struct linked_list_node *next;
} linked_list_node;

typedef struct stack {
    linked_list_node *head;
    int size;
} stack;

stack *create_stack();
void stack_push(stack *, unsigned char);
char stack_pop(stack *);

void _create_huffman_map(char *[256], stack *, huffman_node *);
void create_huffman_map(char *[256], huffman_node *);

int str_len(char *);

typedef struct file_stream_writer {
    FILE *file;
    unsigned char cur_bit;
    unsigned char cur_byte;
} file_stream_writer;

file_stream_writer *create_file_stream_writer(char *);
void write_byte(file_stream_writer *, unsigned char);
void write_bit(file_stream_writer *, unsigned char);
void destroy_file_stream_writer(file_stream_writer *);

void encode(char *, char *);
void decode();

int main() {
    encode("test/file.txt", "output.huff");
    decode();
    return 0;
}

char _decode_dfs(file_stream_reader *reader, huffman_node *node) {
    if (node->left == NULL && node->right == NULL) {
        return node->value;
    }
    if (next_bit(reader) == 0) {
        return _decode_dfs(reader, node->left);
    } else {
        return _decode_dfs(reader, node->right);
    }
}

void decode() {
    file_stream_reader *file_reader = create_file_stream_reader("test/file.txt");

    long occ[256];
    count_occurrences(occ, file_reader);
    reset_file_stream_reader(file_reader);

    int total_byte_count = 0;
    while (peek_byte(file_reader) != EOF) {
        total_byte_count++;
        next_byte(file_reader);
    }

    huffman_node *root = create_huffman_tree(occ);
    file_stream_reader *encoded_file_reader = create_file_stream_reader("output.huff");

    file_stream_writer *decoded_file_writer = create_file_stream_writer("decoded-huff.txt");

    for (int i = 0; i < total_byte_count; i++) {
        char c = _decode_dfs(encoded_file_reader, root);
        write_byte(decoded_file_writer, c);
    }
}

void encode(char *src_path, char *output_path) {
    file_stream_reader *reader = create_file_stream_reader(src_path);

    long occ[256];
    count_occurrences(occ, reader);

    huffman_node *root = create_huffman_tree(occ);

    char *map[256];
    create_huffman_map(map, root);

    // for (int i = 0; i < 256; i++) {
    //     if (*(map + i) != NULL) {
    //         printf("%s, %c\n", *(map + i), i);
    //     }
    // }

    reset_file_stream_reader(reader);
    file_stream_writer *writer = create_file_stream_writer(output_path);
    while (peek_byte(reader) != EOF) {
        unsigned char c = peek_byte(reader);
        char *path = map[c];
        for (int i = 0; i < str_len(path); i++) {
            char digit = *(path + i);
            write_bit(writer, digit);
        }
        next_byte(reader);
    }

    destroy_file_stream_reader(reader);
    destroy_file_stream_writer(writer);
}

file_stream_writer *create_file_stream_writer(char *file_path) {
    file_stream_writer *stream = (file_stream_writer *)malloc(sizeof(file_stream_writer));
    FILE *file;
    file = fopen(file_path, "w");
    stream->file = file;
    stream->cur_bit = 7;
    stream->cur_byte = 0;
    return stream;
}

void write_bit(file_stream_writer *writer, unsigned char bit) {
    if (bit == '1') {
        writer->cur_byte |= (1 << writer->cur_bit);
    }
    if (writer->cur_bit == 0) {
        writer->cur_bit = 7;
        write_byte(writer, writer->cur_byte);
        writer->cur_byte = 0;
    } else {
        writer->cur_bit--;
    }
}

void write_byte(file_stream_writer *writer, unsigned char byte) {
    fwrite(&byte, 1, 1, writer->file);
}

void destroy_file_stream_writer(file_stream_writer *writer) {
    fclose(writer->file);
}

file_stream_reader *create_file_stream_reader(char *file_path) {
    file_stream_reader *stream = (file_stream_reader *)malloc(sizeof(file_stream_reader));
    FILE *file;
    file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "ccct: file %s not found.", file_path);
        exit(0);
    }
    stream->file = file;
    stream->curr = fgetc(file);
    stream->cur_bit = 7;
    return stream;
}

char peek_byte(file_stream_reader *stream) {
    return stream->curr;
}

char next_byte(file_stream_reader *stream) {
    char curr = stream->curr;
    stream->curr = fgetc(stream->file);
    return curr;
}

char peek_bit(file_stream_reader *reader) {
    char bit = reader->curr & (1 << reader->cur_bit);
    return bit != 0 ? 1 : 0;
}

char next_bit(file_stream_reader *reader) {
    char bit = reader->curr & (1 << reader->cur_bit);
    if (reader->cur_bit == 0) {
        reader->cur_bit = 7;
        next_byte(reader);
    } else {
        reader->cur_bit--;
    }
    return bit != 0 ? 1 : 0;
}

void reset_file_stream_reader(file_stream_reader *stream) {
    rewind(stream->file);
    stream->curr = fgetc(stream->file);
}

void destroy_file_stream_reader(file_stream_reader *stream) {
    fclose(stream->file);
}

void count_occurrences(long occ[256], file_stream_reader *stream) {
    for (int i = 0; i < 256; i++) {
        unsigned char c = i;
        *(occ + c) = 0;
    }
    while (peek_byte(stream) != EOF) {
        unsigned char c = next_byte(stream);
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

huffman_node *create_huffman_node(unsigned char c, long freq, huffman_node *left, huffman_node *right) {
    huffman_node *node = NULL;
    node = (huffman_node *)malloc(sizeof(huffman_node));
    node->value = c;
    node->frequency = freq;
    node->left = left;
    node->right = right;
    return node;
}

huffman_node *create_huffman_tree(long occurrences[256]) {
   min_heap *heap = create_heap();
    for (int i = 0; i < 256; i++) {
        unsigned char c = i;
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

stack *create_stack() {
    stack *s = (stack *)malloc(sizeof(stack));
    s->head = NULL;
    s->size = 0;
    return s;
}

void stack_push(stack *s, unsigned char c) {
    linked_list_node *node = (linked_list_node *)malloc(sizeof(linked_list_node));
    node->value = c;
    node->next = s->head;
    s->head = node;
    s->size++;
}

char stack_pop(stack *s) {
    if (s->size == 0) {
        fprintf(stderr, "ccct: stack underflow error.\n");
        exit(0);
    }
    linked_list_node *node = s->head;
    char c = node->value;
    s->head = s->head->next;
    s->size--;
    free(node);
    return c;
}

void _create_huffman_map(char *map[256], stack *s, huffman_node *node) {
    if (node == NULL) {
        return;
    }
    if (node->left == NULL && node->right == NULL) {
        linked_list_node *list_node = s->head;
        char *path = (char *)malloc(s->size + 1);
        int i = 0;
        while (list_node != NULL) {
            *(path + s->size - i - 1) = list_node->value;
            list_node = list_node->next;
            i++;
        }
        *(path + i) = '\0';
        map[node->value] = path;
    }
    stack_push(s, '0');
    _create_huffman_map(map, s, node->left);
    stack_pop(s);
    stack_push(s, '1');
    _create_huffman_map(map, s, node->right);
    stack_pop(s);
}

void create_huffman_map(char *map[256], huffman_node *root) {
    for (int c = 0; c < 256; c++) {
        *(map + c) = NULL;
    }
    stack *s = create_stack();
    _create_huffman_map(map, s, root);
    free(s);
}

int str_len(char *str) {
    int len = 0;
    while (*(str + len) != '\0') {
        len++;
    }
    return len;
}

void print_huffman_node(huffman_node *node) {
    printf("%c, %ld, %p, %p\n", node->value, node->frequency, node->left, node->right);
}
