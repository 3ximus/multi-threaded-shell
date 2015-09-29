# Makefile
par-shell: par-shell.o commandlinereader.o
	gcc -o par-shell par-shell.o commandlinereader.o
par-shell.o: par-shell.c commandlinereader.h
	gcc -Wall -c par-shell.c
commandlinereader.o: commandlinereader.c commandlinereader.h
	gcc -Wall -c commandlinereader.c
clean:
	rm -f par-shell *.o fibonacci
fibonacci:
	gcc -o fibonacci fibonacci.c
