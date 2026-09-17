// Para garantir que o comando make compila o programa corretamente e gera o executável "./user"

CC = gcc
CFLAGS = -Wall -Wextra -g

all: user

user: main.c
	$(CC) $(CFLAGS) main.c -o user

clean:
	rm -f user