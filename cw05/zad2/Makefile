CC = gcc -Wall -std=c11 -ggdb
SEP="========================================="

all: main

test: main
	./main.out google.com
	sleep 2
	./main.out https://www.cplusplus.com/reference/ cplusplus.com 1

dirs:
	@mkdir -p build

main.o: main.c
	$(CC) -c main.c -o build/main.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

main: dirs main.o
	$(CC) build/main.o -o main.out

clean:
	rm -Rf build tests *.com *.out *.a *.so *.o core *~ .idea
