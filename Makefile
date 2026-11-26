CC = gcc
CFLAGS = -std=c11 -g -Wall -Wextra -Iinclude

all: src/repl.c
	$(CC) $(CFLAGS) src/repl.c -o ct.exe

clean:
	rm -f repl.o ct.exe