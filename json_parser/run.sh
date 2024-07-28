mkdir -p bin
clang main.c -o bin/json_parser
./bin/json_parser "$@"
