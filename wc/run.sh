mkdir -p bin
clang main.c -o bin/wc
./bin/wc "$@"
