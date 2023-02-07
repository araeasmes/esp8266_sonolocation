CC = gcc
CFLAGS = -g -Wall
INCLUDES = -I controller/include

default: controller

%.o: controller/%.c
	mkdir -p bin
	$(CC) $(CFLAGS) $(INCLUDES) -o bin/$@ -c $^

controller: main.o net_interface_utils.o
	$(CC) $(CFLAGS) -o bin/controller $(addprefix bin/,$^)

clean:
	$(RM) bin/*

