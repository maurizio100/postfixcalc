CC=gcc
CFLAGS = -ansi -pedantic -Wall -c -g -D_XOPEN_SOURCE=500 
PROG = calc

all: $(PROG)

calc: $(PROG).o
	$(CC) -o $(PROG) $(PROG).o -lm

$(PROG).o: $(PROG).c
	$(CC) $(CFLAGS) $(PROG).c

clean:
	rm -f $(PROG).o $(PROG)

.PHONY: all clean
