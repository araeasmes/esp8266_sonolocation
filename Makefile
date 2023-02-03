CC = gcc
CFLAGS = -g -Wall


bin/controller: controller/main.c
	mkdir -p ./bin
	$(CC) $(CFLAGS) -o ./bin/controller controller/main.c

