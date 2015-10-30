CC=gcc
CFLAGS=-Wall -Weverything -pedantic

par-shell: par-shell.o commandlinereader.o
	$(CC) -o par-shell par-shell.o commandlinereader.o -pthread
	$(CC) -o fibonacci fibonacci.c
par-shell.o: par-shell.c commandlinereader.h
	$(CC) -c par-shell.c
commandlinereader.o: commandlinereader.c commandlinereader.h
	$(CC) -c commandlinereader.c
clean:
	rm -f par-shell *.o fibonacci
