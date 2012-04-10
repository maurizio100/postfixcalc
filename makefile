CC=gcc
CFLAGS = -std=c99 -pedantic -Wall -D_XOPEN_SOURCE=500 -D_BSD_SOURCE -g -c
PROG = calc

all: $(PROG)

calc: $(PROG).o
	$(CC) -o $(PROG) $(PROG).o -lm

$(PROG).o: $(PROG).c
	$(CC) $(CFLAGS) $(PROG).c

clean:
	rm -f $(PROG).o $(PROG)

.PHONY: all clean
