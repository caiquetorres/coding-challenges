#include <stdio.h>
#include <stdlib.h>

#define PAD 8

#define BYTE_COUNT (1 << 0)
#define LINE_COUNT (1 << 1)
#define WORDS_COUNT (1 << 2)
#define CHARS_COUNT (1 << 3)

typedef struct info {
    char *file_name;
    int line_count;
    int words_count;
    int byte_count;
    int chars_count;
} info;

typedef struct char_stream {
    FILE *file;
    char *file_path;
    char curr;
} char_stream;

void run_for_streams(int, char_stream *, int);

char_stream create_char_stream(char *);
char peek_char(char_stream *);
char next_char(char_stream *);
void destroy_char_stream(char_stream *);

char is_option(char *);
int read_options(int, char **);

int str_len(char *);
char str_compare(char *, char *);
char is_whitespace(char);
int count_digits(int);

void print_value(int);
void print_info(info, int);

int main(int argc, char **argv) {
    int options = read_options(argc, argv);
    int i = 1;
    while (i < argc && is_option(*(argv + i))) {
        i++;
    }
    int files_count = argc - i;
    int streams_count;
    char_stream *streams = NULL;
    if (files_count == 0) {
        char_stream stream = create_char_stream(NULL);
        streams = (char_stream *)malloc(sizeof(char_stream));
        *streams = stream;
        streams_count = 1;
    } else {
        char **files = argv + i;
        streams = (char_stream *)malloc(files_count * sizeof(char_stream));
        for (int j = 0; j < files_count; j++) {
            char_stream stream = create_char_stream(*(files + j));
            *(streams + j) = stream;
        }
        streams_count = files_count;
    }
    run_for_streams(streams_count, streams, options);
    for (int j = 0; j < streams_count; j++) {
        destroy_char_stream(streams + j);
    }
    free(streams);
}

void run_for_streams(int count, char_stream *streams, int options) {
    info total_data = { "total", 0, 0, 0, 0 };
    for (int i = 0; i < count; i++) {
        char_stream *stream = streams + i;
        info local_data = { stream->file_path, 0, 0, 0, 0 };
        char prev = EOF;
        char c = next_char(stream);
        while (c != EOF) {
            if (!options || options & LINE_COUNT) {
                if (c == '\n') {
                    local_data.line_count++;
                    total_data.line_count++;
                }
            }
            if (!options || options & WORDS_COUNT) {
                if (is_whitespace(c) && !is_whitespace(prev)) {
                    local_data.words_count++;
                    total_data.words_count++;
                }
            }
            if (!options || options & BYTE_COUNT) {
                local_data.byte_count++;
                total_data.byte_count++;
            }
            if (options & CHARS_COUNT) {
                if ((c & 0xC0) != 0x80) {
                    local_data.chars_count++;
                    total_data.chars_count++;
                }
            }
            prev = c;
            c = next_char(stream);
        }
        if (!options || options & WORDS_COUNT) {
            if (is_whitespace(c) && !is_whitespace(prev)) {
                local_data.words_count++;
                total_data.words_count++;
            }
        }
        print_info(local_data, options);
    }
    if (count > 1) {
        print_info(total_data, options);
    }
}

char_stream create_char_stream(char *file_path) {
    char_stream stream;
    if (file_path) {
        FILE *file = NULL;
        file = fopen(file_path, "r");
        if (file == NULL) {
            fprintf(stderr, "ccwc: file '%s' does not exist\n", file_path);
            exit(0);
        }
        stream.file_path = file_path;
        stream.file = file;
    } else {
        stream.file_path = NULL;
        stream.file = stdin;
    }
    stream.curr = getc(stream.file);
    return stream;
}

char peek_char(char_stream *stream) {
    return stream->curr;
}

char next_char(char_stream *stream) {
    char res = stream->curr;
    stream->curr = fgetc(stream->file);
    return res;
}

void destroy_char_stream(char_stream *stream) {
    fclose(stream->file);
}

char is_option(char *arg) {
    return *arg == '-';
}

int read_options(int argc, char **argv) {
    int options = 0;
    for (int i = 1; i < argc; i++) {
        char *arg = *(argv + i);
        if (!is_option(arg)) {
            continue;
        }
        if (str_compare("-c", arg)) {
            options |= BYTE_COUNT;
        }
        if (str_compare("-l", arg)) {
            options |= LINE_COUNT;
        }
        if (str_compare("-w", arg)) {
            options |= WORDS_COUNT;
        }
        if (str_compare("-m", arg)) {
            options |= CHARS_COUNT;
        }
        if (!options) {
            fprintf(stderr, "ccwc: invalid option '%s'\n", arg);
            exit(0);
        }
    }
    return options;
}

int str_len(char *str) {
    int len = 0;
    while (*(str + len) != '\0') {
        len++;
    }
    return len;
}

char str_compare(char *str1, char *str2) {
    int len1 = str_len(str1);
    int len2 = str_len(str2);
    if (len1 != len2) {
        return 0;
    }
    for (int i = 0; i < len1; i++) {
        if (*(str1 + i) != *(str2 + i)) {
            return 0;
        }
    }
    return 1;
}

char is_whitespace(char c) {
    char whitespaces[] = {' ', '\t', '\n', '\v', '\f', '\r', EOF};
    for (int i = 0; i < 7; i++) {
        if (c == *(whitespaces + i)) {
            return 1;
        }
    }
    return 0;
}

int count_digits(int n) {
    if (n == 0) {
        return 0;
    }
    unsigned int count = 0;
    while (n != 0) {
        count++;
        n = n / 10;
    }
    return count;
}

void print_value(int value) {
    int digits_count = count_digits(value);
    for (int i = 0; i < PAD - digits_count; i++) {
        printf(" ");
    }
    printf("%d", value);
}

void print_info(info data, int options) {
    if (!options || options & LINE_COUNT) {
        print_value(data.line_count);
    }
    if (!options || options & WORDS_COUNT) {
        print_value(data.words_count);
    }
    if (!options || options & BYTE_COUNT) {
        print_value(data.byte_count);
    }
    if (options & CHARS_COUNT) {
        print_value(data.chars_count);
    }
    if (data.file_name != NULL) {
        printf(" %s\n", data.file_name);
    } else {
        printf("\n");
    }
}
