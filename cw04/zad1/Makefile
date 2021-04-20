CC = gcc -Wall -std=c11
SEP="========================================="

all: main

dirs:
	@mkdir -p build

main.o: main.c
	$(CC) -c main.c -o build/main.o -D_POSIX_C_SOURCE

main: dirs main.o
	$(CC) build/main.o -o main.out

exec_main.o: exec_main.c
	$(CC) -c exec_main.c -o build/exec_main.o -D_POSIX_C_SOURCE

exec_main: dirs exec_main.o
	$(CC) build/exec_main.o -o exec_main.out

child.o: child.c
	$(CC) -c child.c -o build/child.o -D_POSIX_C_SOURCE

child: dirs child.o
	$(CC) build/child.o -o child.out

run_fork: main
	@echo $(SEP)
	./main.out ignore
	@echo $(SEP)
	./main.out handler
	@echo $(SEP)
	./main.out mask
	@echo $(SEP)
	./main.out pending
	@echo $(SEP)

run_exec: exec_main child
	@echo $(SEP)
	./exec_main.out ignore
	@echo $(SEP)
	./exec_main.out handler
	@echo $(SEP)
	./exec_main.out mask
	@echo $(SEP)
	./exec_main.out pending
	@echo $(SEP)


clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
