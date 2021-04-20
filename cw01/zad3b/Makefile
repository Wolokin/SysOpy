# Default optimization lvl is -O2,
# To use a different one do: make <target> OPT=<optimization-flag>
# i.e.: make all OPT=-O0
OPT = -O2
CC = gcc -Wall -std=c11 $(OPT)
SEP = "=================================================="

all: static

dirs:
	@mkdir -p build

lib.a: dirs lib.c lib.h
	$(CC) -c lib.c -o build/lib.o
	ar rcs lib_lib.a build/lib.o

lib.so: dirs lib.c lib.h
	$(CC) -c -fPIC lib.c -o build/lib.o
	$(CC) -shared -fPIC build/lib.o -o lib_lib.so

static: lib.a main.c
	$(CC) main.c -L. -l:lib_lib.a -o main.out

static_with_bench: lib.a main.c
	$(CC) main.c -L. -l:lib_lib.a -o main.out -Dbenchmark

shared: lib.so main.c
	$(CC) main.c -L. -l_lib -o main.out -Wl,-rpath=.

shared_with_bench: lib.so main.c
	$(CC) main.c -L. -l_lib -o main.out -Wl,-rpath=. -Dbenchmark

dynamic: lib.so main.c
	$(CC) main.c -ldl -o main.out -DDynamic

dynamic_with_bench: lib.so main.c
	$(CC) main.c -ldl -o main.out -DDynamic -Dbenchmark

test: static
	./main.out 5 merge_files main.c:lib.c lib.c:lib.h remove_block 1 print_merged

static_tests: static_with_bench
	@echo $(SEP)
	@echo "Running tests with a static library"
	@echo $(SEP)
	@make run_tests

shared_tests: shared_with_bench
	@echo $(SEP)
	@echo "Running tests with a shared library"
	@echo $(SEP)
	@make run_tests

dynamic_tests: dynamic_with_bench
	@echo $(SEP)
	@echo "Running tests with a dynamic library"
	@echo $(SEP)
	@make run_tests

opt_info:
	@echo $(SEP)
	@echo $(SEP)$(SEP)
	@echo "Running tests with optimization lvl: "$(OPT)
	@echo $(SEP)$(SEP)
	@echo $(SEP)

bench_all: opt_info static_tests shared_tests dynamic_tests


test_dir:
	@mkdir -p tests

test1: test_dir
	./test_generator.sh 1 5 4 250000

test2: test_dir
	./test_generator.sh 2 50 4 250000

test3: test_dir
	./test_generator.sh 3 500 4 250000

test4: test_dir
	./test_generator.sh 4 5 4 500000

test5: test_dir
	./test_generator.sh 5 5 4 5000000

test6: test_dir
	./test_generator.sh 6 5 4 50000000


run_tests: test1 test2 test3 test4 test5 test6
	@echo "Test1 consisting of 5 file pairs, 250000 characters, 4 per line"
	@./main.out $$(cat tests/test1.txt)
	@echo "Test2 consisting of 50 file pairs, 250000 characters, 4 per line"
	@./main.out $$(cat tests/test2.txt)
	@echo "Test3 consisting of 500 file pairs, 250000 characters, 4 per line"
	@./main.out $$(cat tests/test3.txt)
	@echo "Test4 consisting of 5 file pairs, 500000 characters, 4 per line"
	@./main.out $$(cat tests/test4.txt)
	@echo "Test5 consisting of 5 file pairs, 5000000 characters, 4 per line"
	@./main.out $$(cat tests/test5.txt)
	@echo "Test6 consisting of 5 file pairs, 50000000 characters, 4 per line"
	@./main.out $$(cat tests/test6.txt)
	@echo $(SEP)
	@echo "Test7 consisting of 5 file pairs, 50000000 characters, 4 per line, which are loaded and then freed 5 times"
	@echo $(SEP)
	@./main.out 5 create_sequence $$(cat tests/test6.txt | cut -c 15-) merge_sequence `for i in 1 2 3 4 5; do echo "create_table 5 read_merged_sequence remove_table";done`
	@echo $(SEP)

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
