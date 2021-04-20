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

bench: main_sys.out main_lib.out run_tests

test_dir:
	@mkdir -p tests

test1: test_dir
	./test_generator.sh 50500000 100

run_tests: test1
	@./main_sys.out a tests/t1.txt
	@./main_lib.out a tests/t1.txt

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea *.tmp
