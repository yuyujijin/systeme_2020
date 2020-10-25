CC=gcc
CFLAGS=-W -Wunused-value -Wall -ansi -pedantic -std=c99 
LDFLAGS= -lm

ALL=shell cmds/pwd cmds/ls cmds/cat cmds/mkdir cmds/cd
all: $(ALL)

shell: shell.c cmds/cd
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

%.o: %.c %.h
	$(CC) $(CFLAGS) -o $@ -c $< $(LDFLAGS)

cmds/pwd: cmds/pwd.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/ls: cmds/ls.o tar_manipulation.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/cat: cmds/cat.o tar_manipulation.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/mkdir: cmds/mkdir.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/cd: cmds/cd.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean cleanall

clean:
	rm -rf $(ALL)

cleano: 
	rm -rf *.o

cleanall:clean cleano

