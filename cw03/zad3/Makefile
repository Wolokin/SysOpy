OPT = -O2
CC = gcc -Wall -std=c11 -g $(OPT)
ARGS = . a 2   # Default ARGS

all: main.out

dirs:
	@mkdir -p build

main.o: dirs main.c
	$(CC) -c main.c -o build/main.o -D_DEFAULT_SOURCE

main.out: main.o
	$(CC) build/main.o -o main.out

run: main.out
	./main.out $(ARGS)

gen_tests:
	./generate_test.sh

test: clean gen_tests run

clean:
	rm -Rf build tests test *.out *.a *.so *.o core *~ .idea *.tmp
