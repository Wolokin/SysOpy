CC = gcc -Wall -std=c11
SEP="========================================="

all: main

dirs:
	@mkdir -p build

main.o: main.c
	$(CC) -c main.c -o build/main.o -D_POSIX_C_SOURCE=200809L

main: dirs main.o
	$(CC) build/main.o -o main.out

run: main
	./main.out


clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
