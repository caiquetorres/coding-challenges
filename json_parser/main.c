#include <stdio.h>
#include <stdlib.h>

#define MAX_DEPTH 20
#define FILE_OPTION (1 << 0);

typedef enum token_type {
    LEFT_BRACKET_TOKEN,
    RIGHT_BRACKET_TOKEN,
    LEFT_BRACE_TOKEN,
    RIGHT_BRACE_TOKEN,
    STRING_TOKEN,
    COLON_TOKEN,
    COMMA_TOKEN,
    NULL_TOKEN,
    BOOL_TOKEN,
    NUM_TOKEN,
    EOF_TOKEN,
    BAD_TOKEN,
} token_type;

typedef struct string {
    char *buffer;
    int len;
    int alloc;
} string;

typedef struct token {
    token_type type;
} token;

typedef struct char_stream {
    FILE *file;
    char curr;
} char_stream;

typedef struct token_stream {
    char_stream *char_stream;
    token curr;
} token_stream;

char is_whitespace(char);
char is_numeric(char);
char is_valid_escaping_char(char);
char is_hex(char);

int read_options(int, char **);
char is_option(string *);

string *create_str(char *);
void destroy_str(string *);
void push_c(string *, char);
char compare_str(string *, string *);

char_stream create_char_stream(char *);
char next_char(token_stream *);
char peek_char(token_stream *);
char is_char_stream_end(char_stream *);
void destroy_char_stream(char_stream *);

token get_token(token_stream *);
token_stream create_token_stream(char_stream *);
token next_token(token_stream *);
token peek_token(token_stream *);
char is_token_stream_end(token_stream *);
void destroy_token_stream(token_stream *);
void print_token(token *);

int check_initial_expression(token_stream *);
int check_expression(token_stream *, int);
int check_array_expression(token_stream *, int);
int check_object_expression(token_stream *, int);
int check_object_property(token_stream *, int);

void check_file(char *);

string *null_str;
string *true_str;
string *false_str;

int main(int argc, char **argv) {
    null_str = create_str("null");
    true_str = create_str("true");
    false_str = create_str("false");
    int options = read_options(argc, argv);
    int i = 1;
    while (i < argc && *(*(argv + i)) == '-') {
        i++;
    }
    int files_count = argc - i;
    if (files_count == 0) {
        check_file(NULL);
    } else {
        char **files = argv + i;
        for (int j = 0; j < files_count; j++) {
            check_file(*(files + j));
        }
    }
    destroy_str(null_str);
    destroy_str(true_str);
    destroy_str(false_str);
    return 0;
}

void check_file(char *file_path) {
    char_stream char_stream = create_char_stream(file_path);
    token_stream token_stream = create_token_stream(&char_stream);
    int res = check_initial_expression(&token_stream);
    if (peek_token(&token_stream).type != EOF_TOKEN) {
        res = 0;
    }
    char *file_name = file_path == NULL ? "stdin" : file_path;
    if (res) {
        printf("%s valid\n", file_name);
    } else {
        printf("%s invalid\n", file_name);
    }
    destroy_token_stream(&token_stream);
}

char is_whitespace(char c) {
    char whitespaces[] = { ' ', '\t', '\n', '\v', '\f', '\r' };
    for (int i = 0; i < 6; i++) {
        if (c == *(whitespaces + i)) {
            return 1;
        }
    }
    return 0;
}

char is_numeric(char c) {
    return c >= '0' && c <= '9';
}

char is_valid_escaping_char(char c) {
    char valid_espace_chars[] = { 'b', 'f', 'n', 'r', 't', '\"', '\\', '/' };
    for (int i = 0; i < 8; i++) {
        if (c == *(valid_espace_chars + i)) {
            return 1;
        }
    }
    return 0;
}

char is_hex(char c) {
    return c >= '0' && c <= '9' || c >= 'a' && c < 'f' || c >= 'A' && c <= 'F';
}

char is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

string *create_str(char *init) {
    int len = 0;
    while (*(init + len) != '\0') {
        len++;
    }
    int alloc = (len + 101) / 100;
    string *str = NULL;
    str = (string *)malloc(sizeof(string));
    str->len = len;
    str->buffer = (char *)malloc(alloc * 100);
    str->alloc = alloc;
    for (int i = 0; i < len; i++) {
        *(str->buffer + i) = *(init + i);
    }
    *(str->buffer + len) = '\0';
    return str;
}

void destroy_str(string *str) {
    free(str->buffer);
    free(str);
}

void push_c(string *str, char c) {
    if (str->len + 2 > str->alloc * 100) {
        str->alloc++;
        str->buffer = (char *)realloc(str->buffer, str->alloc * 100);
    }
    *(str->buffer + str->len) = c;
    str->len++;
    *(str->buffer + str->len) = '\0';
}

char compare_str(string *str1, string *str2) {
    if (str1->len != str2->len) {
        return 0;
    }
    for (int i = 0; i < str1->len; i++) {
        if (*(str1->buffer + i) != *(str2->buffer + i)) {
            return 0;
        }
    }
    return 1;
}

char_stream create_char_stream(char *file_path) {
    char_stream stream;
    if (file_path != NULL) {
        FILE *file = NULL;
        file = fopen(file_path, "r");
        if (file == NULL) {
            fprintf(stderr, "ccjsonparser: file '%s' does not exist\n", file_path);
            exit(0);
        }
        stream.file = file;
    } else {
        stream.file = stdin;
    }
    stream.curr = fgetc(stream.file);
    return stream;
}

char next_char(token_stream *stream) {
    char c = stream->char_stream->curr;
    stream->char_stream->curr = fgetc(stream->char_stream->file);
    return c;
}

char peek_char(token_stream *stream) {
    return stream->char_stream->curr;
}

char is_char_stream_end(char_stream *stream) {
    return stream->curr == EOF;
}

void destroy_char_stream(char_stream *stream) {
    fclose(stream->file);
}

token get_token(token_stream *ts) {
    while (is_whitespace(peek_char(ts))) {
        next_char(ts);
    }
    token token;
    switch (peek_char(ts)) {
        case '{':
            next_char(ts);
            token.type = LEFT_BRACKET_TOKEN;
            break;
        case '}':
            next_char(ts);
            token.type = RIGHT_BRACKET_TOKEN;
            break;
        case '[':
            next_char(ts);
            token.type = LEFT_BRACE_TOKEN;
            break;
        case ']':
            next_char(ts);
            token.type = RIGHT_BRACE_TOKEN;
            break;
        case '"':
            next_char(ts); // "
            while (peek_char(ts) != EOF && peek_char(ts) != '"') {
                if (peek_char(ts) == '\n' || peek_char(ts) == '\t') {
                    token.type = BAD_TOKEN;
                    return token;
                }
                if (peek_char(ts) == '\\')  {
                    next_char(ts); // backslash
                    if (peek_char(ts) == 'u') {
                        next_char(ts); // u
                        for (int i = 0; i < 4; i++) {
                            if (!is_hex(peek_char(ts))) {
                                token.type = BAD_TOKEN;
                                return token;
                            }
                            next_char(ts);
                        }
                    } else if (!is_valid_escaping_char(peek_char(ts))) {
                        token.type = BAD_TOKEN;
                        return token;
                    }
                }
                next_char(ts); // "
            }
            if (next_char(ts) == EOF) {
                token.type = BAD_TOKEN;
            } else {
                token.type = STRING_TOKEN;
            }
            break;
        case ':':
            next_char(ts);
            token.type = COLON_TOKEN;
            break;
        case ',':
            next_char(ts);
            token.type = COMMA_TOKEN;
            break;
        case EOF:
            next_char(ts);
            token.type = EOF_TOKEN;
            break;
        default:
            if (is_alpha(peek_char(ts))) {
                string *keyword = create_str("");
                while (is_alpha(peek_char(ts))) {
                    char c = next_char(ts);
                    push_c(keyword, c);
                }
                if (compare_str(keyword, null_str)) {
                    token.type = NULL_TOKEN;
                } else if (compare_str(keyword, true_str) || compare_str(keyword, false_str)) {
                    token.type = BOOL_TOKEN;
                }
                destroy_str(keyword);
            } else if (is_numeric(peek_char(ts)) || peek_char(ts) == '-') {
                if (peek_char(ts) == '-') {
                    next_char(ts); // -
                }
                if (peek_char(ts) == '0') {
                    next_char(ts); // 0
                    if (is_numeric(peek_char(ts))) {
                        token.type = BAD_TOKEN;
                        return token;
                    }
                }
                while (is_numeric(peek_char(ts))) {
                    next_char(ts); // num
                }
                if (peek_char(ts) == '.') {
                    next_char(ts); // .
                    if (!is_numeric(peek_char(ts))) {
                        token.type = BAD_TOKEN;
                        return token;
                    }
                    while (is_numeric(peek_char(ts))) {
                        next_char(ts); // num
                    }
                }
                if (peek_char(ts) == 'e' || peek_char(ts) == 'E') {
                    next_char(ts); // e, E
                    if (peek_char(ts) == '+' || peek_char(ts) == '-') {
                        next_char(ts); // -, +
                    }
                    if (!is_numeric(peek_char(ts))) {
                        token.type = BAD_TOKEN;
                        return token;
                    }
                    while (is_numeric(peek_char(ts))) {
                        next_char(ts); // num
                    }
                }
                token.type = NUM_TOKEN;
            } else {
               token.type = BAD_TOKEN;
            }
            break;
    }
    return token;
}

token_stream create_token_stream(char_stream *char_stream) {
    token_stream stream;
    stream.char_stream = char_stream;
    stream.curr = get_token(&stream);
    return stream;
}

token next_token(token_stream *token_stream) {
    token t = token_stream->curr;
    token_stream->curr = get_token(token_stream);
    return t;
}

token peek_token(token_stream *token_stream) {
    return token_stream->curr;
}

void destroy_token_stream(token_stream *token_stream) {
    destroy_char_stream(token_stream->char_stream);
}

void print_token(token *token) {
    switch (token->type) {
        case LEFT_BRACKET_TOKEN:
            printf("LEFT BRACKET TOKEN\n");
            break;
        case RIGHT_BRACKET_TOKEN:
            printf("RIGHT BRACKET TOKEN\n");
            break;
        case RIGHT_BRACE_TOKEN:
            printf("RIGHT BRACE TOKEN\n");
            break;
        case LEFT_BRACE_TOKEN:
            printf("LEFT BRACE TOKEN\n");
            break;
        case COLON_TOKEN:
            printf("COLON TOKEN\n");
            break;
        case STRING_TOKEN:
            printf("STRING TOKEN\n");
            break;
        case COMMA_TOKEN:
            printf("COMMA TOKEN\n");
            break;
        case NULL_TOKEN:
            printf("NULL TOKEN\n");
            break;
        case NUM_TOKEN:
            printf("NUM TOKEN\n");
            break;
        case BOOL_TOKEN:
            printf("BOOL TOKEN\n");
            break;
        case EOF_TOKEN:
            printf("EOF TOKEN\n");
            break;
        default:
            printf("BAD TOKEN\n");
            break;
    }
}

int check_initial_expression(token_stream *token_stream) {
    token token = peek_token(token_stream);
    if (token.type == LEFT_BRACE_TOKEN) {
        return check_array_expression(token_stream, 1);
    } else if (token.type == LEFT_BRACKET_TOKEN) {
        return check_object_expression(token_stream, 1);
    } else {
        return 0;
    }
}

int check_expression(token_stream *token_stream, int depth) {
    token token = peek_token(token_stream);
    if (token.type == NULL_TOKEN) {
        next_token(token_stream);
        return 1;
    } else if (token.type == NUM_TOKEN) {
        next_token(token_stream);
        return 1;
    } else if (token.type == BOOL_TOKEN) {
        next_token(token_stream);
        return 1;
    } else if (token.type == STRING_TOKEN) {
        next_token(token_stream);
        return 1;
    } else if (token.type == LEFT_BRACE_TOKEN) {
        return check_array_expression(token_stream, depth);
    } else if (token.type == LEFT_BRACKET_TOKEN) {
        return check_object_expression(token_stream, depth);
    } else {
        return 0;
    }
}

int check_array_expression(token_stream *token_stream, int depth) {
    depth++;
    if (depth > MAX_DEPTH) {
        return 0;
    }
    token left_brace = next_token(token_stream); // [
    if (left_brace.type != LEFT_BRACE_TOKEN) {
        return 0;
    }
    int res = 1;
    while (peek_token(token_stream).type != RIGHT_BRACE_TOKEN) {
        res = res && check_expression(token_stream, depth);
        if (!res) {
            return 0;
        }
        token comma_token = peek_token(token_stream);
        if (comma_token.type != COMMA_TOKEN && comma_token.type != RIGHT_BRACE_TOKEN) {
            return 0;
        }
        if (comma_token.type == COMMA_TOKEN) {
            next_token(token_stream); // ,
            if (peek_token(token_stream).type == RIGHT_BRACE_TOKEN) {
                return 0;
            }
        }
    }
    token right_brace = next_token(token_stream); // ]
    if (right_brace.type != RIGHT_BRACE_TOKEN) {
        return 0;
    }
    return 1;
}

int check_object_expression(token_stream *token_stream, int depth) {
    depth++;
    if (depth > MAX_DEPTH) {
        return 0;
    }
    token left_bracket = next_token(token_stream); // {
    if (left_bracket.type != LEFT_BRACKET_TOKEN) {
        return 0;
    }
    int res = 1;
    while (peek_token(token_stream).type != RIGHT_BRACKET_TOKEN) {
        res = res && check_object_property(token_stream, depth);
        if (!res) {
            return 0;
        }
        token comma_token = peek_token(token_stream);
        if (comma_token.type != COMMA_TOKEN && comma_token.type != RIGHT_BRACKET_TOKEN) {
            return 0;
        }
        if (comma_token.type == COMMA_TOKEN) {
            next_token(token_stream); // ,
            if (peek_token(token_stream).type == RIGHT_BRACKET_TOKEN) {
                return 0;
            }
        }
    }
    if (!res) {
        return 0;
    }
    token right_bracket = next_token(token_stream); // }
    if (right_bracket.type != RIGHT_BRACKET_TOKEN) {
        return 0;
    }
    return 1;
}

int check_object_property(token_stream *token_stream, int depth) {
    token key_token = next_token(token_stream); // "key"
    if (key_token.type != STRING_TOKEN) {
        return 0;
    }
    token colon_token = next_token(token_stream); // :
    if (colon_token.type != COLON_TOKEN) {
        return 0;
    }
    return check_expression(token_stream, depth);
}

char is_option(string *arg) {
    return arg->len > 0 && *(arg->buffer) == '-';
}

int read_options(int argc, char **argv) {
    int options = 0;
    string *file_option = create_str("-f");
    string *file_option_lg = create_str("--file");
    for (int i = 1; i < argc; i++) {
        string *arg = create_str(*(argv + i));
        if (!is_option(arg)) {
            continue;
        }
        if (compare_str(arg, file_option) || compare_str(arg, file_option_lg)) {
            options |= FILE_OPTION;
        } else {
            fprintf(stderr, "ccjsonparser: invalid option '%s'\n", arg->buffer);
        }
        destroy_str(arg);
    }
    destroy_str(file_option);
    destroy_str(file_option_lg);
    return options;
}
