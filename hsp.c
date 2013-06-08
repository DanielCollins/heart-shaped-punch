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

int sock;
struct sockaddr_in out_addr, in_addr;
socklen_t out_addr_len, in_addr_len;

void udp_readable_cb(EV_P_ ev_io *w, int revents)
{
  socklen_t bytes_read;
  char buffer[1001];
  (void) loop;
  (void) w;
  (void) revents;
  bytes_read = recvfrom(sock, buffer, 1000, 0, (struct sockaddr*) &out_addr,         
   (socklen_t*) &out_addr_len);
  buffer[bytes_read] = '\0';
  printf(": %s\n", buffer);
}

void spam(char *host)
{
  struct addrinfo hints, *servinfo;
  int rv;
  char buffer[] = "spam";

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  if ((rv = getaddrinfo(host, PORT, &hints, &servinfo)) != 0)
  {
    fprintf(stderr, "gettaddrifo: %s\n", gai_strerror(rv));
    return;
  }
  if (!servinfo) return;
  sendto(sock, buffer, strlen(buffer), 0, servinfo->ai_addr,
           servinfo->ai_addrlen);
  freeaddrinfo(servinfo);
}

int main(int argc, char *argv[])
{
  ev_io udp_watcher;
  struct ev_loop *loop;

  in_addr_len = sizeof(in_addr);
  out_addr_len = sizeof(out_addr);
  sock = socket(PF_INET, SOCK_DGRAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  bzero(&in_addr, in_addr_len);
  in_addr.sin_family = AF_INET;
  in_addr.sin_port = htons(42000);
  in_addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr*) &in_addr, in_addr_len) != 0)
    fprintf(stderr, "could not bind\n");

  if (argc == 2)
    spam(argv[1]);

  loop = ev_default_loop(0);
  ev_io_init(&udp_watcher, udp_readable_cb, sock, EV_READ);
  ev_io_start(loop, &udp_watcher);

  ev_run(loop, 0);

  return EXIT_SUCCESS;
}


