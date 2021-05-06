CC = gcc -Wall -std=c11 -ggdb
SEP="========================================="

all: manager cook uber

dirs:
	@mkdir -p build

manager.o: manager.c
	$(CC) -c manager.c -o build/manager.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

manager: dirs manager.o
	$(CC) build/manager.o -o manager.out -lrt

cook.o: cook.c
	$(CC) -c cook.c -o build/cook.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

cook: dirs cook.o
	$(CC) build/cook.o -o cook.out -lrt

uber.o: uber.c
	$(CC) -c uber.c -o build/uber.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

uber: dirs uber.o
	$(CC) build/uber.o -o uber.out -lrt

run: all
	./manager.out 5 5

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
