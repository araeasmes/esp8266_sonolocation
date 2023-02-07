CC = gcc
CFLAGS = -g -Wall

SRCS := main.c net_interface_utils.c 
INCLUDES := -I include

bin/controller: bin/main.o bin/net_interface_utils.o
	$(CC) $(CFLAGS) -o bin/controller $^ 
	
bin/%.o: %.c
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ -c $^

clean:
	$(RM) bin/*

