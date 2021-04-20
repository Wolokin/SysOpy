CC = gcc -Wall -std=c11

all: main

dirs:
	@mkdir -p build

lib.a: dirs lib.c lib.h
	$(CC) -c lib.c -o build/lib.o
	ar rcs build/lib_lib.a build/lib.o

main.o: main.c
	$(CC) -c main.c -o build/main.o

main: dirs lib.a main.o
	$(CC) build/main.o -Lbuild -l_lib -o main.out

test: main
	./main.out 5 merge_files main.c:lib.c lib.c:lib.h remove_block 1 print_merged

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


bench: dirs main.c lib.a test1 test2 test3 test4 test5 test6
	$(CC) -c -Dbenchmark main.c -o build/main.o
	$(CC) build/main.o -l_lib -Lbuild -o bench.out
	@echo "Test1 consisting of 5 file pairs, 250000 characters, 4 per line"
	@./bench.out $$(cat tests/test1.txt)
	@echo "Test2 consisting of 50 file pairs, 250000 characters, 4 per line"
	@./bench.out $$(cat tests/test2.txt)
	@echo "Test3 consisting of 500 file pairs, 250000 characters, 4 per line"
	@./bench.out $$(cat tests/test3.txt)
	@echo "Test4 consisting of 5 file pairs, 500000 characters, 4 per line"
	@./bench.out $$(cat tests/test4.txt)
	@echo "Test5 consisting of 5 file pairs, 5000000 characters, 4 per line"
	@./bench.out $$(cat tests/test5.txt)
	@echo "Test6 consisting of 5 file pairs, 50000000 characters, 4 per line"
	@./bench.out $$(cat tests/test6.txt)
	@echo "Test7 consisting of 5 file pairs, 50000000 characters, 4 per line, which are loaded and then freed 5 times"
	@echo "=========================================="
	@./bench.out 5 create_sequence $$(cat tests/test6.txt | cut -c 15-) merge_sequence `for i in 1 2 3 4 5; do echo "create_table 5 read_merged_sequence remove_table";done`
	@echo "=========================================="

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
