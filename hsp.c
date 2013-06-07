#include <stdlib.h>
#include <stdio.h>
#include <ev.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/unistd.h>
#include <sys/fcntl.h>
#include <strings.h>

int sock;
struct sockaddr_in addr;
int addr_len;

void udp_readable_cb(EV_P_ ev_io *w, int revents)
{
  socklen_t bytes_read;
  char buffer[1001];
  (void) loop;
  (void) w;
  (void) revents;
  bytes_read = recvfrom(sock, buffer, 1000, 0, (struct sockaddr*) &addr,         
   (socklen_t*) &addr_len);
  buffer[bytes_read] = '\0';
  printf(": %s\n", buffer);
}

int main(void)
{
  ev_io udp_watcher;
  struct ev_loop *loop;
  addr_len = sizeof(addr);
  sock = socket(PF_INET, SOCK_DGRAM, 0);
  fcntl(sock, F_SETFL, O_NONBLOCK);
  bzero(&addr, addr_len);
  addr.sin_family = AF_INET;
  addr.sin_port = htons(42000);
  addr.sin_addr.s_addr = INADDR_ANY;
  if (bind(sock, (struct sockaddr*) &addr, addr_len) != 0)
    fprintf(stderr, "could not bind\n");

  loop = ev_default_loop(0);
  ev_io_init(&udp_watcher, udp_readable_cb, sock, EV_READ);
  ev_io_start(loop, &udp_watcher);

  ev_run(loop, 0);

  return EXIT_SUCCESS;
}


