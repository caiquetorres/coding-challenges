mkdir -p bin
clang main.c -o bin/compression_tool
./bin/compression_tool "$@"
