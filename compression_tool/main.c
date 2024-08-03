#include <stdio.h>
#include <stdlib.h>

#define MAX_HEAP_SIZE 256

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

typedef struct file_stream_reader {
    FILE *file;
    char byte;
    char bit;
} file_stream_reader;

file_stream_reader *create_file_stream_reader(char *);
char next_byte(file_stream_reader *);
char peek_byte(file_stream_reader *);
char next_bit(file_stream_reader *);
char peek_bit(file_stream_reader *);
void reset_file_stream_reader(file_stream_reader *);
void destroy_file_stream_reader(file_stream_reader *);

typedef struct file_stream_writer {
    FILE *file;
    unsigned char bit;
    unsigned char byte;
} file_stream_writer;

file_stream_writer *create_file_stream_writer(char *);
void write_bit(file_stream_writer *, int);
void write_byte(file_stream_writer *, char);
void write_bytes(file_stream_writer *, char *, unsigned int);
void write_number(file_stream_writer *, long int);
void destroy_file_stream_writer(file_stream_writer *);

void count_occurrences(long[256], file_stream_reader *);

typedef struct huffman_node {
    unsigned char value;
    long frequency;
    struct huffman_node *left;
    struct huffman_node *right;
} huffman_node;

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

huffman_node *create_huffman_node(unsigned char, long, huffman_node *, huffman_node *);
huffman_node *create_huffman_tree(long [256]);
void create_huffman_map(char *[256], huffman_node *);
void _fill_huffman_map(char *[256], stack *, huffman_node *);

int str_len(char *);
void print_binary(unsigned char);
char is_numeric(char);

void encode(char *, char *);

void decode(char *, char *);
long int _decode_number(file_stream_reader *);
char _decode_huff(file_stream_reader *, huffman_node *);

int main() {
    encode("test/book.txt", "out/output.huff");
    decode("out/output.huff", "out/decoded.txt");
    return 0;
}

void encode(char *src_path, char *out_path) {
    long int occ[256] = { 0 };
    char *map[256] = { NULL };
    unsigned int src_path_len = str_len(src_path);
    int i = src_path_len;
    while (i >= 0 && *(src_path + i) != '/') {
        i--;
    }
    char *file_name = src_path + i + 1;
    file_stream_reader *src_reader = create_file_stream_reader(src_path);
    file_stream_writer *out_writer = create_file_stream_writer(out_path);
    while (peek_byte(src_reader) != EOF) {
        unsigned char c = next_byte(src_reader);
        *(occ + c) += 1;
    }
    reset_file_stream_reader(src_reader);
    huffman_node *root = create_huffman_tree(occ);
    create_huffman_map(map, root);
    write_bytes(out_writer, "HUFF;", 5);
    write_bytes(out_writer, file_name, str_len(file_name));
    write_byte(out_writer, ';');
    int has_prev = 0;
    for (int i = 0; i < 256; i++) {
        unsigned char c = i;
        int freq = *(occ + i);
        if (freq == 0) {
            continue;
        }
        if (has_prev) {
            write_byte(out_writer, ',');
        }
        has_prev = 1;
        write_byte(out_writer, c);
        write_byte(out_writer, ':');
        write_number(out_writer, freq);
    }
    write_byte(out_writer, ';');
    while (peek_byte(src_reader) != EOF) {
        unsigned char c = peek_byte(src_reader);
        char *path = map[c];
        int path_len = str_len(path);
        for (int i = 0; i < path_len; i++) {
            char digit = *(path + i);
            write_bit(out_writer, digit - '0');
        }
        next_byte(src_reader);
    }
    destroy_file_stream_reader(src_reader);
    destroy_file_stream_writer(out_writer);
}

void decode(char *src_path, char *out_path) {
    long int occ[256];
    long int total_byte_count = 0;
    file_stream_reader *src_reader = create_file_stream_reader(src_path);
    file_stream_writer *out_writer = create_file_stream_writer(out_path);
    char *file_type = "HUFF;";
    int i = 0;
    do {
        if (*(file_type + i) != next_byte(src_reader)) {
            fprintf(stderr, "ccct: expected a HUFF file\n");
            exit(0);
        }
        i++;
    } while (peek_byte(src_reader) != ';');
    next_byte(src_reader); // ;
    while (peek_byte(src_reader) != ';') {
        next_byte(src_reader);
    }
    next_byte(src_reader); // ;
    char is_first = 1;
    do {
        if (!is_first) {
            next_byte(src_reader); // ,
        }
        is_first = 0;
        unsigned char c = next_byte(src_reader);
        next_byte(src_reader); // :
        long int freq = _decode_number(src_reader);
        total_byte_count += freq;
        occ[c] = freq;
    } while (peek_byte(src_reader) != ';');
    next_byte(src_reader); // ;
    huffman_node *root = create_huffman_tree(occ);
    for (int i = 0; i < total_byte_count; i++) {
        char c = _decode_huff(src_reader, root);
        write_byte(out_writer, c);
    }
    destroy_file_stream_reader(src_reader);
    destroy_file_stream_writer(out_writer);
}

long int _decode_number(file_stream_reader *reader) {
    long int ans = 0;
    while (is_numeric(peek_byte(reader))) {
        char digit = next_byte(reader) - '0';
        ans += digit;
        ans *= 10;
    }
    ans /= 10;
    return ans;
}

char _decode_huff(file_stream_reader *reader, huffman_node *node) {
    if (node->left == NULL && node->right == NULL) {
        return node->value;
    }
    if (next_bit(reader) == 0) {
        return _decode_huff(reader, node->left);
    } else {
        return _decode_huff(reader, node->right);
    }
}

file_stream_writer *create_file_stream_writer(char *file_path) {
    file_stream_writer *stream = (file_stream_writer *)malloc(sizeof(file_stream_writer));
    FILE *file;
    file = fopen(file_path, "w");
    stream->file = file;
    stream->bit = 7;
    stream->byte = 0;
    return stream;
}

void write_bit(file_stream_writer *writer, int bit) {
    if (bit == 1) {
        writer->byte |= (1 << writer->bit);
    }
    if (writer->bit == 0) {
        write_byte(writer, writer->byte);
        writer->bit = 7;
        writer->byte = 0;
    } else {
        writer->bit--;
    }
}

void write_byte(file_stream_writer *writer, char byte) {
    fwrite(&byte, 1, 1, writer->file);
}

void write_bytes(file_stream_writer *writer, char *bytes, unsigned int count) {
    fwrite(bytes, 1, count, writer->file);
}

void write_number(file_stream_writer *writer, long int number) {
    if (number == 0) {
        write_byte(writer, '0');
    }
    stack *s = create_stack();
    while (number != 0) {
        stack_push(s, number % 10 + '0');
        number /= 10;
    }
    while (s->size > 0) {
        char c = stack_pop(s);
        write_byte(writer, c);
    }
    free(s);
}

void destroy_file_stream_writer(file_stream_writer *writer) {
    fclose(writer->file);
}

file_stream_reader *create_file_stream_reader(char *file_path) {
    file_stream_reader *stream = (file_stream_reader *)malloc(sizeof(file_stream_reader));
    FILE *file;
    file = fopen(file_path, "r");
    if (file == NULL) {
        fprintf(stderr, "ccct: file %s not found.\n", file_path);
        exit(0);
    }
    stream->file = file;
    stream->byte = fgetc(file);
    stream->bit = 7;
    return stream;
}

char peek_byte(file_stream_reader *stream) {
    return stream->byte;
}

char next_byte(file_stream_reader *stream) {
    char curr = stream->byte;
    stream->byte = fgetc(stream->file);
    return curr;
}

char peek_bit(file_stream_reader *reader) {
    char bit = reader->byte & (1 << reader->bit);
    return bit != 0 ? 1 : 0;
}

char next_bit(file_stream_reader *reader) {
    char bit = reader->byte & (1 << reader->bit);
    if (reader->bit == 0) {
        reader->bit = 7;
        next_byte(reader);
    } else {
        reader->bit--;
    }
    return bit != 0 ? 1 : 0;
}

void reset_file_stream_reader(file_stream_reader *stream) {
    rewind(stream->file);
    stream->byte = fgetc(stream->file);
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

void _fill_huffman_map(char *map[256], stack *s, huffman_node *node) {
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
    _fill_huffman_map(map, s, node->left);
    stack_pop(s);
    stack_push(s, '1');
    _fill_huffman_map(map, s, node->right);
    stack_pop(s);
}

void create_huffman_map(char *map[256], huffman_node *root) {
    stack *s = create_stack();
    _fill_huffman_map(map, s, root);
    free(s);
}

int str_len(char *str) {
    int len = 0;
    while (*(str + len) != '\0') {
        len++;
    }
    return len;
}

char is_numeric(char c) {
    return c >= '0' && c <= '9';
}
