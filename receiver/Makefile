CC = gcc
CFLAGS = -g -Wall
CLIBS = -lm

SRCS := main.c
INCLUDES := -I include -I ../common/include 
HEADERS := ../common/include/espnow_packet.h \
		   ../common/include/payload.h \
		   ../common/include/vecmath.h \
			include/localizator.h

bin/receiver: bin/main.o
	$(CC) $(CFLAGS) -o bin/receiver $^ $(CLIBS)

bin/%.o: %.c $(HEADERS) 
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) $< -c -o $@ $(CLIBS)

clean:
	$(RM) bin/*

