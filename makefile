CC = gcc
CFLAGS = -Wextra -lrt

all: oss10 worker

oss: oss.c
        $(CC) $(CFLAGS) -o oss oss.c -lrt

worker: worker.c
        $(CC) $(CFLAGS) -o worker worker.c

clean:
        rm -f oss worker
