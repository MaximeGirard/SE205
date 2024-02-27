CC = gcc
CFLAGS = -pedantic -Werror -Wall -std=c11 -pthread -O3 -g

all: main_mutex

main_mutex: mutex.o td2.4_mutex.o
	$(CC) $(CFLAGS) -o $@ $^

mutex.o: mutex.c
	$(CC) $(CFLAGS) -c $<

td2.4_mutex.o: td2.4_mutex.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f main_mutex *.o
