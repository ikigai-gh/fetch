CC = gcc
CFLAGS = -std=c99 -O0 -g -Wall -Wpedantic

fetch: fetch.c
	$(CC) $(CFLAGS) $(LDFLAGS) fetch.c -o fetch
