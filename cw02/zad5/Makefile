OPT = -O2
CC = gcc -Wall -std=c11 -g $(OPT)
SEP = "=================================================="

all: main_sys.out main_lib.out

dirs:
	@mkdir -p build

main_lib.o: dirs main_lib.c
	$(CC) -c main_lib.c -o build/main_lib.o

main_lib.out: main_lib.o
	$(CC) build/main_lib.o -o main_lib.out

main_sys.o: dirs main_sys.c
	$(CC) -c main_sys.c -o build/main_sys.o

main_sys.out: main_sys.o
	$(CC) build/main_sys.o -o main_sys.out

run: main_sys.out
	./main_sys.out

bench: clean main_sys.out main_lib.out run_tests

test_dir:
	@mkdir -p tests

test1: test_dir
	./test_generator.sh 49999975 70

run_tests: test1
	@./main_sys.out tests/t1.txt tests/t2.txt a b
	@./main_lib.out tests/t1.txt tests/t2.txt a b

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea *.tmp
