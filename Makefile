CC = gcc
CFLAGS = -g -Wall
INCLUDES = controller/include

all: receiver controller

controller: controller_dummy
	make -C controller

controller_dummy: ;

receiver: receiver_dummy
	make -C receiver

receiver_dummy: ;

clean:
	make -C controller clean
	make -C receiver clean
