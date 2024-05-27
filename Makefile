libbnums.a: big_numbers.o
	ar -cvq libbnums.a big_numbers.o
big_numbers.o: big_numbers.c big_numbers.h
	gcc -Wall -g -c big_numbers.c
clean:
	rm -f big_numbers.o libbnums.a
