CC = gcc
CFLAGS = -g -Wall

SRCS := main.c
INCLUDES := -I include -I ../common/include 
HEADERS := ../common/include/espnow_packet.h

bin/receiver: bin/main.o
	$(CC) $(CFLAGS) -o bin/receiver $^

bin/%.o: %.c $(HEADERS) 
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $< -c -o $@

clean:
	$(RM) bin/*

