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

struct Peer
{
  struct sockaddr addr;
  socklen_t addr_len;
  enum HostState state;
  struct Peer *next;
};

int sock;
struct sockaddr_in listen_addr;
socklen_t listen_addr_len;
struct Peer *peers;

struct Peer *new_peer()
{
  struct Peer *result;
  if (!(result = malloc(sizeof(struct Peer))))
  {
    fprintf(stderr, "error: malloc failed\n");
    return 0;
  }
  result->next = peers;
  peers = result;
  result->state = DEAD;
  result->addr_len = sizeof(result->addr);
  bzero(&result->addr, result->addr_len);
  return result;
}

int addr_equal(struct sockaddr *a, socklen_t a_len,
   struct sockaddr *b, socklen_t b_len)
{
  if (a_len != b_len) return 0;
  return memcmp(a, b, a_len) == 0;
}

struct Peer *get_peer(struct sockaddr *addr, socklen_t len)
{
  struct Peer *result;
  for (result = peers; result; result = result->next)
    if (addr_equal(&result->addr, result->addr_len, addr, len))
      return result;
  return 0;
}

void acknowledge(struct Peer *p)
{
  char b = ACK;
  sendto(sock, &b, 1, 0, &p->addr, p->addr_len);
}

void establish(struct Peer *p)
{ 
  char b = EST;
  sendto(sock, &b, 1, 0, &p->addr, p->addr_len);      
}

void _connect(struct Peer *p)
{ 
  char b = CON;
  sendto(sock, &b, 1, 0, &p->addr, p->addr_len);      
}

void udp_readable_cb(EV_P_ ev_io *w, int revents)
{
  char b;
  struct sockaddr src;
  socklen_t len;
  struct Peer *p;
  (void) loop;
  (void) w;
  (void) revents;
  len = sizeof(src);
  recvfrom(sock, &b, 1, 0, &src, &len);
  p = get_peer(&src, len);
  switch (b)
  {
    case CON:
      printf("client wants to connect\n");
      if (!p)
      {
        p = new_peer();
        memcpy(&p->addr, &src, len);
        p->state = ACKNOWLEDGING;
        acknowledge(p);
      }
      else if (p->state == DEAD)
      {
        acknowledge(p);
        p->state = ACKNOWLEDGING;
      }  
      return;
    case ACK:
      if (p->state == CONNECTING || p->state == ESTABLISHED)
      {
        establish(p);
        p->state = ESTABLISHED;
        printf("established\n");
      }
      return;
    case EST:
      printf("established\n");
      p->state = ESTABLISHED;
      return;
  }
  fprintf(stderr, "warning: bad packet\n");
}

void lookup(char *host, struct Peer *p)
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
  memcpy(&p->addr, servinfo->ai_addr, servinfo->ai_addrlen);
  p->addr_len = servinfo->ai_addrlen;
/*  freeaddrinfo(servinfo); */
}

int main(int argc, char *argv[])
{
  ev_io udp_watcher;
  struct ev_loop *loop;
  struct Peer *p;

  p = 0;
  listen_addr_len = sizeof(listen_addr);
  sock = socket(PF_INET, SOCK_DGRAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  bzero(&listen_addr, listen_addr_len);
  listen_addr.sin_family = AF_INET;
  listen_addr.sin_port = htons(42000);
  listen_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr*) &listen_addr, listen_addr_len) != 0)
    fprintf(stderr, "could not bind\n");

  if (argc == 2)
  {
    p = new_peer();
    lookup(argv[1], p);
    p->state = CONNECTING;
    _connect(p);
  }

  loop = ev_default_loop(0);
  ev_io_init(&udp_watcher, udp_readable_cb, sock, EV_READ);
  ev_io_start(loop, &udp_watcher);

  ev_run(loop, 0);

  return EXIT_SUCCESS;
}


