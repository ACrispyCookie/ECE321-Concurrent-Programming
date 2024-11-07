./target/test: ./target/test.o ./target/mysem.o
	gcc -Wall -g ./target/test.o ./target/mysem.o -o ./target/test

./target/test.o: ./test/test_mysem.c ./src/mysem.h
	gcc -Wall -g -c ./test/test_mysem.c -o ./target/test.o

./target/mysem.o: ./src/mysem.c ./src/mysem.h
	gcc -Wall -g -c ./src/mysem.c -o ./target/mysem.o