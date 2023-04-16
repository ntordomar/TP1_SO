CC = gcc
CFLAGS = -Wall -Wextra -pedantic -std=c99

all: application worker

application: application.o
	$(CC) $(CFLAGS) -o $@ $^

application.o: application.c
	$(CC) $(CFLAGS) -c $<

worker:
	$(CC) $(CFLAGS) -o $@ $^	

worker.o: worker.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f application *.o
