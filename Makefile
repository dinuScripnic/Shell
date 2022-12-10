CC = gcc
CFLAGS = -D_DEFAULT_SOURCE -Wall -Wextra -Werror -pedantic -std=c17 -g 

.PHONY: all
all:
	$(CC) $(CFLAGS) -o mine mine.c

.PHONY: clean
clean:
	rm -f mine

insecure:
	$(CC) -o mine mine.c