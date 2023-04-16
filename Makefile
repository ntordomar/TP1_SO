CC = gcc
CFLAGS = -Wall -lrt -pthread

all: application view worker

application: application.c information.c
	$(CC) $(CFLAGS) application.c information.c -o application

view: view.c information.c
	$(CC) $(CFLAGS) view.c information.c -o view

worker: worker.c
	$(CC) $(CFLAGS) worker.c -o worker

clean:
	rm -f application view worker