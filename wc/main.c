#include <stdio.h>
#include <stdlib.h>

#define true 1;
#define false 0;

#define PAD 8

#define BYTE_COUNT (1 << 0)
#define LINE_COUNT (1 << 1)
#define WORDS_COUNT (1 << 2)
#define CHARS_COUNT (1 << 3)

typedef char bool;

struct info {
    char *name;
    unsigned int line_count;
    unsigned int words_count;
    unsigned int byte_count;
    unsigned int chars_count;
};

unsigned int str_len(char *);
unsigned int count_digits(unsigned int);
bool str_compare(char *, char *);
bool is_whitespace(char);

bool is_option(char *);
unsigned int read_options(int, char **);

char *read_file_contents(char *);
char *read_from_stdin(int);

unsigned int count_bytes(int, char *);
unsigned int count_lines(int, char *);
unsigned int count_words(int, char *);
unsigned int count_chars(int, char *);

void run_for_stdin(unsigned int);
void run_for_files(int, char **, unsigned int);

void print_value(unsigned int);
void print_info(struct info, unsigned int);

int main(int argc, char **argv) {
    unsigned int options = read_options(argc, argv);
    int i = 1;
    while (i < argc && is_option(*(argv + i))) {
        i++;
    }
    // i has the first non-option argument (in this case is a file).
    unsigned int files_count = argc - i;
    if (files_count == 0) {
        run_for_stdin(options);
    } else {
        char **files = argv + i;
        run_for_files(files_count, files, options);
    }
    return 0;
}

unsigned int str_len(char *str) {
    int len = 0;
    while (*(str + len) != '\0') {
        len++;
    }
    return len;
}

unsigned int count_digits(unsigned int n) {
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

bool is_whitespace(char c) {
    char whitespaces[] = { ' ', '\t', '\n', '\v', '\f', '\r', EOF };
    for (int i = 0; i < 7; i++) {
        if (c == *(whitespaces + i)) {
            return true;
        }
    }
    return false;
}

bool str_compare(char *str1, char *str2) {
    int len1 = str_len(str1);
    int len2 = str_len(str2);
    if (len1 != len2) {
        return false;
    }
    for (int i = 0; i < len1; i++) {
        if (*(str1 + i) != *(str2 + i)) {
            return false;
        }
    }
    return true;
}

bool is_option(char *arg) {
    return *arg == '-';
}

unsigned int read_options(int argc, char **argv) {
    unsigned int options = 0;
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
            fprintf(stderr, "Invalid option '%s'\n", arg);
            exit(0);
        }
    }
    return options;
}

char *read_file_contents(char *file_path) {
    char *buffer = NULL;
    unsigned int len = 0;
    FILE *fptr = NULL;
    fptr = fopen(file_path, "r");
    if (fptr == NULL) {
        fprintf(stderr, "File '%s' does not exist\n", file_path);
        exit(0);
    }
    fseek(fptr, 0, SEEK_END);
    len = ftell(fptr);
    fseek(fptr, 0, SEEK_SET);
    buffer = (char *)malloc(len);
    if (buffer != NULL) {
        fread(buffer, 1, len, fptr);
        fclose(fptr);
    } else {
        fprintf(stderr, "No memory left");
        fclose(fptr);
        exit(0);
    }
    return buffer;
}

char *read_from_stdin(int step) {
    char *contents;
    contents = (char *)malloc(step);
    int len = 0;
    int mult = 1;
    char curr;
    while ((curr = getc(stdin)) != EOF) {
        if (len + 1 > step * mult) {
            mult++;
            contents = (char *)realloc(contents, step * mult);
        }
        *(contents + len) = curr;
        len++;
    }
    contents = (char *)realloc(contents, len + 1);
    *(contents + len) = '\0';
    return contents;
}

unsigned int count_lines(int contents_count, char *contents) {
    unsigned int count = 0;
    for (int i = 0; i < contents_count; i++) {
        if (*(contents + i) == '\n') {
            count++;
        }
    }
    return count;
}

unsigned int count_words(int contents_count, char *contents) {
    unsigned int count = 0;
    char curr;
    char prev = *contents;
    for (int i = 0; i < contents_count; i++) {
        curr = *(contents + i);
        if (is_whitespace(curr) && !is_whitespace(prev)) {
            count++;
        }
        prev = curr;
    }
    if (is_whitespace(curr) && !is_whitespace(prev)) {
        count++;
    }
    return count;
}

unsigned int count_chars(int contents_count, char *contents) {
    unsigned int count = 0;
    for (int i = 0; i < contents_count; i++) {
        if ((*(contents + i) & 0xC0) != 0x80) {
            count++;
        }
    }
    return count;
}

void run_for_stdin(unsigned int options) {
    char *contents = read_from_stdin(100);
    unsigned int contents_count = str_len(contents);
    struct info info;
    info.name = "";
    if (!options || options & LINE_COUNT) {
        info.line_count = count_lines(contents_count, contents);
    }
    if (!options || options & WORDS_COUNT) {
        info.words_count = count_words(contents_count, contents);
    }
    if (!options || options & BYTE_COUNT) {
        info.byte_count = contents_count;
    }
    if (options & CHARS_COUNT) {
        info.chars_count = count_chars(contents_count, contents);
    }
    print_info(info, options);
}

void run_for_files(int files_count, char **files, unsigned int options) {
    struct info total_info = { "total", 0, 0, 0, 0 };
    for (int i = 0; i < files_count; i++) {
        char *contents = read_file_contents(*(files + i));
        unsigned int contents_count = str_len(contents);
        struct info info;
        info.name = *(files + i);
        if (!options || options & LINE_COUNT) {
            info.line_count = count_lines(contents_count, contents);
            total_info.line_count += info.line_count;
        }
        if (!options || options & WORDS_COUNT) {
            info.words_count = count_words(contents_count, contents);
            total_info.words_count += info.words_count;
        }
        if (!options || options & BYTE_COUNT) {
            info.byte_count = contents_count;
            total_info.byte_count += info.byte_count;
        }
        if (options & CHARS_COUNT) {
            info.chars_count = count_chars(contents_count, contents);
            total_info.chars_count += info.chars_count;
        }
        print_info(info, options);
    }
    if (files_count > 1) {
        print_info(total_info, options);
    }
}

void print_info(struct info info, unsigned int options) {
    if (!options || options & LINE_COUNT) {
        print_value(info.line_count);
    }
    if (!options || options & WORDS_COUNT) {
        print_value(info.words_count);
    }
    if (!options || options & BYTE_COUNT) {
        print_value(info.byte_count);
    }
    if (options & CHARS_COUNT) {
        print_value(info.chars_count);
    }
    if (str_len(info.name) != 0) {
        printf(" %s\n", info.name);
    } else {
        printf("\n");
    }
}

void print_value(unsigned int value) {
    unsigned int digits_count = count_digits(value);
    for (int i = 0; i < PAD - digits_count; i++) {
        printf(" ");
    }
    printf("%d", value);
}
