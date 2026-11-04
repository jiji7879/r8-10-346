CC      ?= cc
CFLAGS  ?= -O2 -std=c11 -Wall
PROGS    = count verify verify2 enum

all: $(PROGS)

count: count.c
	$(CC) $(CFLAGS) -o $@ $<
verify: verify.c
	$(CC) $(CFLAGS) -o $@ $<
verify2: verify2.c
	$(CC) $(CFLAGS) -o $@ $<
enum: enum.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(PROGS)

.PHONY: all clean
