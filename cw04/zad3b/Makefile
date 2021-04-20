CC = gcc -Wall -std=c11
SEP="========================================="

all: catcher sender

dirs:
	@mkdir -p build

catcher.o: catcher.c
	$(CC) -c catcher.c -o build/catcher.o -D_POSIX_C_SOURCE=200809L

catcher: dirs catcher.o
	$(CC) build/catcher.o -o catcher.out

sender.o: sender.c
	$(CC) -c sender.c -o build/sender.o -D_POSIX_C_SOURCE=200809L

sender: dirs sender.o
	$(CC) build/sender.o -o sender.out

run: all
	./sender.out KILL 10


clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
