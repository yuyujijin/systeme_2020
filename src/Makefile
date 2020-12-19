CC=gcc
CFLAGS=-W -Wunused-value -Wall -ansi -pedantic -std=c99
LDFLAGS= -lm -g

ALL=shell cmds/pwd_tar cmds/ls_tar cmds/cat_tar cmds/mkdir_tar
all: $(ALL)

shell: tar.c tar_manipulation.c cmds/cd.c shell.c useful.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/pwd_tar: cmds/pwd.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/ls_tar: tar.c tar_manipulation.c cmds/ls.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/cat_tar: tar.c tar_manipulation.c cmds/cat.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/mkdir_tar: tar.c tar_manipulation.c cmds/mkdir.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean cleanall

clean:
	rm -rf $(ALL)

cleano:
	rm -rf *.o cmds/*.o

cleanall:clean cleano
