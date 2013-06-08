#include <stdlib.h>
#include <stdio.h>
#include <ev.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <strings.h>
#include <netdb.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define PORT "42000"

#define CON 0
#define ACK 1
#define EST 2

enum HostState
{
  DEAD,
  CONNECTING,
  ACKNOWLEDGING,
  ESTABLISHED
};

int sock;
struct sockaddr_in out_addr, in_addr;
struct sockaddr client_addr;
socklen_t out_addr_len, in_addr_len, client_addr_len;
enum HostState state;

void acknowledge(void)
{
  char b = ACK;
  sendto(sock, &b, 1, 0, &client_addr, client_addr_len);
}

void establish(void)
{ 
  char b = EST;
  sendto(sock, &b, 1, 0, &client_addr, client_addr_len);      
}

void _connect(void)
{ 
  char b = CON;
  sendto(sock, &b, 1, 0, &client_addr, client_addr_len);      
}

void udp_readable_cb(EV_P_ ev_io *w, int revents)
{
  char b;
  (void) loop;
  (void) w;
  (void) revents;
  recvfrom(sock, &b, 1, 0, &client_addr, &client_addr_len);
  switch (b)
  {
    case CON:
      if (state == DEAD || state == CONNECTING)
      {
        acknowledge();
        state = ACKNOWLEDGING;
        printf("client wants to connect\n");
      }  
      return;
    case ACK:
      if (state == CONNECTING || state == ESTABLISHED)
      {
        establish();
        state = ESTABLISHED;
        printf("established\n");
      }
      return;
    case EST:
      printf("established\n");
      state = ESTABLISHED;
      return;
  }
  fprintf(stderr, "warning: bad packet\n");
}

void lookup(char *host)
{
  struct addrinfo hints, *servinfo;
  int rv;
  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  if ((rv = getaddrinfo(host, PORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "gettaddrinfo: %s\n", gai_strerror(rv));
    return;
  }
  if (!servinfo) return;
  memcpy(&client_addr, servinfo->ai_addr, servinfo->ai_addrlen);
  client_addr_len = servinfo->ai_addrlen;
/*  freeaddrinfo(servinfo); */
}

int main(int argc, char *argv[])
{
  ev_io udp_watcher;
  struct ev_loop *loop;

  state = DEAD;
  in_addr_len = sizeof(in_addr);
  out_addr_len = sizeof(out_addr);
  client_addr_len = sizeof(client_addr);
  sock = socket(PF_INET, SOCK_DGRAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  bzero(&in_addr, in_addr_len);
  in_addr.sin_family = AF_INET;
  in_addr.sin_port = htons(42000);
  in_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr*) &in_addr, in_addr_len) != 0)
    fprintf(stderr, "could not bind\n");

  state = DEAD;

  if (argc == 2)
  {
    lookup(argv[1]);
    state = CONNECTING;
    _connect();
  }

  loop = ev_default_loop(0);
  ev_io_init(&udp_watcher, udp_readable_cb, sock, EV_READ);
  ev_io_start(loop, &udp_watcher);

  ev_run(loop, 0);

  return EXIT_SUCCESS;
}


