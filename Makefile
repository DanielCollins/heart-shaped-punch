CFLAGS += -I/usr/local/include -pedantic -Wall -Wextra -Werror -pipe
LDFLAGS += -L/usr/local/lib -lev

all: hsp

hsp: hsp.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o hsp hsp.c

clean:
	rm -f hsp

