CC = gcc
CFLAGS = -Wall -Wextra -g
LDFLAGS = -lm

SRCS = Client_TCP.c Serv_TCP.c
OBJS = $(SRCS:.c=.o)
EXEC_CLIENT = client
EXEC_SERV = serv

.PHONY: all clean

all: $(EXEC_CLIENT) $(EXEC_SERV)

$(EXEC_CLIENT): Client_TCP.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

$(EXEC_SERV): Serv_TCP.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(EXEC_CLIENT) $(EXEC_SERV)
