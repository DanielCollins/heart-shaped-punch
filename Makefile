CFLAGS += -pedantic -Wall -Wextra -Werror -pipe
LDFLAGS += -lev

all: hsp

hsp: hsp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o hsp hsp.c

clean:
	rm -f hsp

