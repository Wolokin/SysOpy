CC = gcc -Wall -std=c11 -ggdb
SEP="========================================="

all: main

dirs:
	@mkdir -p build

main.o: main.c
	$(CC) -c main.c -o build/main.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

main: dirs main.o
	$(CC) build/main.o -o main.out -lpthread

run: all
	./main.out 1 block fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 2 block fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 4 block fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 8 block fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 16 block fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 1 numbers fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 2 numbers fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 4 numbers fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 8 numbers fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt
	./main.out 16 numbers fractal_tree.ascii.pgm fractal_tree.ascii_negative.pgm >> Times.txt


clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
