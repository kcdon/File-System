
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)

CC = gcc
CFLAGS = -Wall -Werror -pedantic -pthread -g -std=c99

EXEC = exec

all: $(OBJS)
	cc -o $(EXEC) $^ $(CFLAGS)

clean:
	rm -f $(OBJS)

fclean:
	rm -f $(OBJS) $(EXEC)

re: fclean all


