#
# Sistemas Operativos 2015-2016
# Grupo 82
# 63572 Pedro Carneiro
# 76959 Fábia Almeida
# 79764 César Alcobia
# 
# Makefile
# Last modified: 2015-11-06 04:26:37

CC=gcc
CFLAGS=-Wall -Wextra -pedantic

all: par-shell fibonacci
par-shell: par-shell.c commandlinereader.o list.o errorhandling.o
	$(CC) $(CFLAGS) -o par-shell par-shell.c commandlinereader.o list.o errorhandling.o
commandlinereader.o: commandlinereader.c
	$(CC) $(CFLAGS) -c commandlinereader.c
list.o: list.c
	$(CC) $(CFLAGS) -c list.c
errorhandling.o: errorhandling.c
	$(CC) $(CFLAGS) -c errorhandling.c
fibonacci: fibonacci.c
	$(CC) $(CFLAGS) -o fibonacci fibonacci.c
debug: par-shell.c commandlinereader.o list.o errorhandling.o
	$(CC) $(CFLAGS) -g -o par-shell par-shell.c commandlinereader.o list.o errorhandling.o
clean:
	rm -f par-shell *.o fibonacci
