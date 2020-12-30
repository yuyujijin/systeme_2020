CC=gcc
CFLAGS=-W -Wunused-value -Wall -ansi -pedantic -std=c99
LDFLAGS= -lm -g

ALL=shell cmds/pwd cmds/ls cmds/cat cmds/mkdir cmds/rmdir cmds/cp cmds/mv cmds/rm unit_test

all: $(ALL)

shell: tar.c tar_manipulation.c cmds/cd.c shell.c useful.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

unit_test: tar.c tar_manipulation.c unit_test.c cmds/cd.c useful.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/pwd: cmds/pwd.o


cmds/mv: cmds/mv.o useful.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/ls: tar.c tar_manipulation.c cmds/ls.o useful.c cmds/cd.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/cat: tar.c tar_manipulation.c cmds/cat.o useful.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/mkdir: tar.c tar_manipulation.c cmds/mkdir.o useful.c cmds/cd.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/cp: tar.c tar_manipulation.c cmds/cp.o useful.c cmds/cd.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/rmdir : tar.c tar_manipulation.c cmds/rmdir.o useful.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cmds/rm : tar.c tar_manipulation.c cmds/rm.o useful.c
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean cleanall

clean:
	rm -rf $(ALL)

cleano:
	rm -rf *.o cmds/*.o

cleanall:clean cleano
