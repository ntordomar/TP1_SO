CC = gcc
CFLAGS = -Wall -lrt -pthread

all: application view worker

application: application.c lib.c ipc_utils.c
	$(CC) $(CFLAGS) application.c lib.c ipc_utils.c -o md5

view: view.c lib.c ipc_utils.c
	$(CC) $(CFLAGS) view.c lib.c ipc_utils.c -o vista

worker: worker.c lib.c
	$(CC) $(CFLAGS) worker.c lib.c -o worker

clean:
	rm -f application view worker