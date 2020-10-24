CC=gcc
CFLAGS=-W -Wall -ansi -pedantic
LDFLAGS= -lm

ALL=shell pwd
all: $(ALL)

shell: shell.c
	$(CC) -o $@ $^ $(LDFLAGS)

pwd: pwd.o
	$(CC) -o $@ $^ $(LDFLAGS)
clean:
	rm -rf *.o

mrproper: clean
	rm -rf $(EXEC)
