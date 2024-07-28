#include <stdio.h>
#include <stdlib.h>

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
    unsigned int len;
    unsigned int mult;
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

string *create_str(char *);
void push_c(string *, char);
char compare_str(string *, string *);

char_stream create_char_stream(char *);
char next_char(char_stream *);
char peek_char(char_stream *);
char is_char_stream_end(char_stream *);
void destroy_char_stream(char_stream *);

token get_token(token_stream *);
token_stream create_token_stream(char_stream *);
token next_token(token_stream *);
token peek_token(token_stream *);
char is_token_stream_end(token_stream *);
void destroy_token_stream(token_stream *);
void print_token(token *);

unsigned int parse_initial_expression(token_stream *);
unsigned int parse_expression(token_stream *);
unsigned int parse_array_expression(token_stream *);
unsigned int parse_object_expression(token_stream *);
unsigned int parse_object_property(token_stream *);

void check_file(char *);

string *null_str;
string *true_str;
string *false_str;

int main(int argc, char **argv) {
    null_str = create_str("null");
    true_str = create_str("true");
    false_str = create_str("false");
    check_file("tests/large-file.json");
    return 0;
}

void check_file(char *file_path) {
    char_stream char_stream = create_char_stream(file_path);
    token_stream token_stream = create_token_stream(&char_stream);
    unsigned int res = parse_initial_expression(&token_stream);
    if (peek_token(&token_stream).type != EOF_TOKEN) {
        token t = peek_token(&token_stream);
        res = 0;
    }
    if (res) {
        printf("%s valid\n", file_path);
    } else {
        printf("%s invalid\n", file_path);
    }
}

char is_whitespace(char c) {
    char whitespaces[] = {' ', '\t', '\n', '\v', '\f', '\r' };
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

char is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

string *create_str(char *init) {
    unsigned int len = 0;
    while (*(init + len) != '\0') {
        len++;
    }
    unsigned int mult = (len + 100) / 100;
    string *str = NULL;
    str = (string *)malloc(sizeof(string));
    str->len = len;
    str->buffer = (char *)malloc(mult * 100);
    str->mult = mult;
    for (int i = 0; i < len; i++) {
        *(str->buffer + i) = *(init + i);
    }
    return str;
}

void push_c(string *str, char c) {
    if (str->len + 2 > str->mult * 100) {
        str->mult++;
        str->buffer = (char *)realloc(str->buffer, str->mult * 100);
    }
    *(str->buffer + str->len) = c;
    *(str->buffer + str->len + 1) = '\0';
    str->len++;
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
    FILE *file = NULL;
    file = fopen(file_path, "r");
    char c = fgetc(file);
    char_stream stream;
    stream.file = file;
    stream.curr = c;
    return stream;
}

char next_char(char_stream *stream) {
    char c = stream->curr;
    stream->curr = fgetc(stream->file);
    return c;
}

char peek_char(char_stream *stream) {
    return stream->curr;
}

char is_char_stream_end(char_stream *stream) {
    return stream->curr == EOF;
}

void destroy_char_stream(char_stream *stream) {
    fclose(stream->file);
}

token get_token(token_stream *token_stream) {
    char c = next_char(token_stream->char_stream);
    while (is_whitespace(c)) {
       c = next_char(token_stream->char_stream);
    }
    token token;
    switch (c) {
        case '{':
            token.type = LEFT_BRACKET_TOKEN;
            break;
        case '}':
            token.type = RIGHT_BRACKET_TOKEN;
            break;
        case '[':
            token.type = LEFT_BRACE_TOKEN;
            break;
        case ']':
            token.type = RIGHT_BRACE_TOKEN;
            break;
        case '"':
            c = next_char(token_stream->char_stream); // "
            while (c != EOF && c != '"') {
                if (c == '\\') {
                    next_char(token_stream->char_stream);
                }
                c = next_char(token_stream->char_stream);
            }
            token.type = c == EOF ? BAD_TOKEN : STRING_TOKEN;
            break;
        case ':':
            token.type = COLON_TOKEN;
            break;
        case ',':
            token.type = COMMA_TOKEN;
            break;
        case EOF:
            token.type = EOF_TOKEN;
            break;
        default:
            if (is_alpha(c)) {
                string *keyword = create_str("");
                push_c(keyword, c);
                while (is_alpha(peek_char(token_stream->char_stream))) {
                    push_c(keyword, next_char(token_stream->char_stream));
                }
                if (compare_str(keyword, null_str)) {
                    token.type = NULL_TOKEN;
                } else if (compare_str(keyword, true_str) || compare_str(keyword, false_str)) {
                    token.type = BOOL_TOKEN;
                } else {
                    token.type = BAD_TOKEN;
                }
                return token;
            } else if (is_numeric(c)) {
                while (is_numeric(peek_char(token_stream->char_stream))) {
                    next_char(token_stream->char_stream);
                }
                if (peek_char(token_stream->char_stream) == '.') {
                    next_char(token_stream->char_stream); // .
                    if (!is_numeric(peek_char(token_stream->char_stream))) {
                        token.type = BAD_TOKEN;
                        return token;
                    }
                    while (is_numeric(peek_char(token_stream->char_stream))) {
                        next_char(token_stream->char_stream);
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
    token ans = token_stream->curr;
    token_stream->curr = get_token(token_stream);
    return ans;
}

token peek_token(token_stream *token_stream) {
    return token_stream->curr;
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

unsigned int parse_initial_expression(token_stream *token_stream) {
    token token = peek_token(token_stream);
    if (token.type == LEFT_BRACE_TOKEN) {
        return parse_array_expression(token_stream);
    } else if (token.type == LEFT_BRACKET_TOKEN) {
        return parse_object_expression(token_stream);
    } else {
        return 0;
    }
}

unsigned int parse_expression(token_stream *token_stream) {
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
        return parse_array_expression(token_stream);
    } else if (token.type == LEFT_BRACKET_TOKEN) {
        return parse_object_expression(token_stream);
    } else {
        return 0;
    }
}

unsigned int parse_array_expression(token_stream *token_stream) {
    token left_BRACE = next_token(token_stream); // [
    if (left_BRACE.type != LEFT_BRACE_TOKEN) {
        return 0;
    }
    unsigned int res = 1;
    while (peek_token(token_stream).type != RIGHT_BRACE_TOKEN) {
        res = res && parse_expression(token_stream);
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
    token right_BRACE = next_token(token_stream); // ]
    if (right_BRACE.type != RIGHT_BRACE_TOKEN) {
        return 0;
    }
    return 1;
}

unsigned int parse_object_expression(token_stream *token_stream) {
    token left_BRACKET = next_token(token_stream); // {
    if (left_BRACKET.type != LEFT_BRACKET_TOKEN) {
        return 0;
    }
    unsigned int res = 1;
    while (peek_token(token_stream).type != RIGHT_BRACKET_TOKEN) {
        res = res && parse_object_property(token_stream);
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
    token right_BRACKET = next_token(token_stream); // }
    if (right_BRACKET.type != RIGHT_BRACKET_TOKEN) {
        return 0;
    }
    return 1;
}

unsigned int parse_object_property(token_stream *token_stream) {
    token key_token = next_token(token_stream); // "key"
    if (key_token.type != STRING_TOKEN) {
        return 0;
    }
    token colon_token = next_token(token_stream); // :
    if (colon_token.type != COLON_TOKEN) {
        return 0;
    }
    return parse_expression(token_stream);
}
