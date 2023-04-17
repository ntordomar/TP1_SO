CC = gcc
CFLAGS = -Wall -lrt -pthread

all: application view worker

application: application.c lib.c ipc_utils.c
	$(CC) $(CFLAGS) application.c lib.c ipc_utils.c -o application

view: view.c lib.c ipc_utils.c
	$(CC) $(CFLAGS) view.c lib.c ipc_utils.c -o view

worker: worker.c
	$(CC) $(CFLAGS) worker.c -o worker

clean:
	rm -f application view worker