CC = gcc -Wall -std=c11 -ggdb
SEP="========================================="

all: server client

dirs:
	@mkdir -p build

server.o: server.c
	$(CC) -c server.c -o build/server.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

server: dirs server.o
	$(CC) build/server.o -o server.out -lrt

client.o: client.c
	$(CC) -c client.c -o build/client.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

client: dirs client.o
	$(CC) build/client.o -o client.out -lrt

run: all
	./panel.sh

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
