OPT = -O2
CC = gcc -Wall -std=c11 -g $(OPT)
ARGS = 5   # Default ARGS

all: main.out

dirs:
	@mkdir -p build

main.o: dirs main.c
	$(CC) -c main.c -o build/main.o

main.out: main.o
	$(CC) build/main.o -o main.out

run: main.out
	./main.out $(ARGS)

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea *.tmp
