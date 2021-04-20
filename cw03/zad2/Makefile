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
	./main.out 5 merge_files main.c:lib.c lib.c:lib.h

test_dir:
	@mkdir -p tests

tests/test1.txt: test_dir
	./test_generator.sh 1 5 4 250000

tests/test2.txt: test_dir
	./test_generator.sh 2 50 4 250000

tests/test3.txt: test_dir
	./test_generator.sh 3 500 4 250000

tests/test4.txt: test_dir
	./test_generator.sh 4 5 4 500000

tests/test5.txt: test_dir
	./test_generator.sh 5 5 4 5000000

tests/test6.txt: test_dir
	./test_generator.sh 6 5 4 50000000

bench.out: dirs main.c lib.a
	$(CC) -c -Dbenchmark main.c -o build/main.o
	$(CC) build/main.o -l_lib -Lbuild -o bench.out

run_tests:
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

light_bench: bench.out run_tests

bench: bench.out tests/test1.txt tests/test2.txt tests/test3.txt tests/test4.txt tests/test5.txt tests/test6.txt run_tests

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
