CC = gcc -Wall -std=c11

all: lib.a lib.so

dirs:
	@mkdir -p build

lib.a: dirs lib.c
	$(CC) -c lib.c -o build/lib.o
	ar rcs lib.a build/lib.o

lib.so: dirs lib.c
	$(CC) -c -fPIC lib.c -o build/lib.o
	$(CC) -shared -fPIC -o lib.so build/lib.o

test: dirs clean lib.c test.c
	@$(CC) -g lib.c test.c -o test.out

clean:
	rm -Rf build *.out *.a *.so *.o core *~ .idea
