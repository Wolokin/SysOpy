CC = gcc -Wall -std=c11 -ggdb
SEP="========================================="

all: producer consumer verifier

test: all test1 test2 test3

dirs:
	@mkdir -p build

producer.o: producer.c
	$(CC) -c producer.c -o build/producer.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

producer: dirs producer.o
	$(CC) build/producer.o -o producer.out

consumer.o: consumer.c
	$(CC) -c consumer.c -o build/consumer.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

consumer: dirs consumer.o
	$(CC) build/consumer.o -o consumer.out

verifier.o: verifier.c
	$(CC) -c verifier.c -o build/verifier.o -D_DEFAULT_SOURCE #-D_POSIX_C_SOURCE=200809L

verifier: dirs verifier.o
	$(CC) build/verifier.o -o verifier.out

test1:
	@echo $(SEP)
	@echo $(SEP)$(SEP)
	@echo "Running test 1, multiple producers, single receiver"
	@echo $(SEP)$(SEP)
	@echo $(SEP)
	@echo "N=5"
	@echo $(SEP)
	./tester.sh 5 100
	@echo $(SEP)
	@echo "N=100"
	@echo $(SEP)
	./tester.sh 100 100
	@echo $(SEP)
	@echo "N=8192"
	@echo $(SEP)
	./tester.sh 8192 100

test2:
	@echo $(SEP)
	@echo $(SEP)$(SEP)
	@echo "Running test 2, single producer, multiple receiver"
	@echo $(SEP)$(SEP)
	@echo $(SEP)
	@echo "N=5"
	@echo $(SEP)
	./tester.sh 5 1 50
	@echo $(SEP)
	@echo "N=100"
	@echo $(SEP)
	./tester.sh 100 1 50
	@echo $(SEP)
	@echo "N=8192"
	@echo $(SEP)
	./tester.sh 8192 1 50

test3:
	@echo $(SEP)
	@echo $(SEP)$(SEP)
	@echo "Running test 3, multiple producers, multiple receivers"
	@echo $(SEP)$(SEP)
	@echo $(SEP)
	@echo "N=5"
	@echo $(SEP)
	./tester.sh 5 100 5
	@echo $(SEP)
	@echo "N=100"
	@echo $(SEP)
	./tester.sh 100 100 5
	@echo $(SEP)
	@echo "N=8192"
	@echo $(SEP)
	./tester.sh 8192 100 5

clean:
	rm -Rf build tests *.out *.a *.so *.o core *~ .idea
