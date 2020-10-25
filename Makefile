CC=gcc		
CFLAGS= -Wall -g
LDLIBS= -lm

EXEC=shell 
all : $(EXEC)

shell:src/shell.c
	$(CC) -o $@ src/shell.c

clean:
	rm -rf *~ $(ALL)
