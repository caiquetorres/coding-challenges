mkdir -p bin
clang main.c -o bin/cut
./bin/cut "$@"
