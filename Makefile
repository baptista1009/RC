CC = gcc
CFLAGS = -Wall -Wextra -g -MMD -MP \
         -I. -Ivalidate -Iargs -Inet -Iauth -Iresources

SRCS = main/main.c $(wildcard validate/*.c args/*.c net/*.c auth/*.c resources/*.c)
OBJS = $(SRCS:.c=.o)

all: user

user: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o user

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f user $(OBJS) $(OBJS:.o=.d)

-include $(OBJS:.o=.d)