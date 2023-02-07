CC = gcc
CFLAGS = -g -Wall

SRCS := main.c
INCLUDES := -I include 

bin/receiver: bin/main.o
	$(CC) $(CFLAGS) -o bin/receiver $^

bin/%.o: %.c
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $^

clean:
	$(RM) bin/*

