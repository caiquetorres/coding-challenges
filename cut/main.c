#include <stdio.h>
#include <stdlib.h>

const size_t BUFFER_SIZE = 1024;

typedef struct file_stream_reader {
	size_t buffer_pos;
	size_t buffer_size;
	char buffer[BUFFER_SIZE];
	FILE *file;
} file_stream_reader;

file_stream_reader *create_file_stream_reader(char *);
char peek_byte(file_stream_reader *);
char next_byte(file_stream_reader *);
void _fill(file_stream_reader *);

int main() {
	file_stream_reader *reader = create_file_stream_reader("main.c");
	while (peek_byte(reader) != EOF) {
		printf("%c", peek_byte(reader));
		next_byte(reader);
	}
	return 0;
}

file_stream_reader *create_file_stream_reader(char *file_path) {
	FILE *file = fopen(file_path, "r");
	if (file == NULL) {
		fprintf(stderr, "cccut: file \"%s\" does not exist\n", file_path);
		exit(1);
	}
	file_stream_reader *reader = (file_stream_reader *)malloc(sizeof(file_stream_reader));
	reader->file = file;
	_fill(reader);
	return reader;
}

char peek_byte(file_stream_reader *reader) {
	if (reader->buffer_pos + 1 > reader->buffer_size) {
		_fill(reader);
	}
	if (reader->buffer_size == 0) {
	   	// If an error occurs or the end of the file is reached,
	   	// return EOF. This signals that the reading process should stop.
		return EOF;
	}
	return reader->buffer[reader->buffer_pos];
}

char next_byte(file_stream_reader *reader) {
	char byte = peek_byte(reader);
	reader->buffer_pos++;
	return byte;
}

void _fill(file_stream_reader *reader) {
	reader->buffer_size = fread(reader->buffer, sizeof(char), BUFFER_SIZE, reader->file);
	reader->buffer_pos = 0;
}
