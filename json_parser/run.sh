mkdir -p bin
clang main.c -o bin/json_parser
./bin/json_parser "$@"

# ./run.sh --file test/fail1.json test/fail2.json test/fail3.json test/fail4.json test/fail5.json test/fail6.json test/fail7.json test/fail8.json test/fail9.json test/fail10.json test/fail11.json test/fail12.json test/fail13.json test/fail14.json test/fail15.json test/fail16.json test/fail17.json test/fail18.json test/fail19.json test/fail20.json test/fail21.json test/fail22.json test/fail23.json test/fail24.json test/fail25.json test/fail26.json test/fail27.json test/fail28.json test/fail29.json test/fail30.json test/fail31.json test/fail32.json test/fail33.json test/pass1.json test/pass2.json test/pass3.json
