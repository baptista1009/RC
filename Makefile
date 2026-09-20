
CC = gcc
CFLAGS = -Wall -Wextra -g

all: user

user: main.c
	$(CC) $(CFLAGS) main.c -o user

clean:
	rm -f user