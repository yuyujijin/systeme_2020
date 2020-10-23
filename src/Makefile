CC=gcc
CFLAGS= -Wall
LDFLAGS=

ALL=mkdir
all: $(ALL)

mkdir: mkdir.o tar_manipulation.o
	gcc -o mkdir mkdir.o tar_manipulation.o

mkdir.o: mkdir.c 
	gcc -o mkdir.o -c mkdir.c 

tar_manipulation.o: tar_manipulation.c
	gcc -o tar_manipulation.o -c tar_manipulation.c

clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
