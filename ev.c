#include <stdlib.h>
#include <stdio.h>
#include <ev.h>

ev_io stdin_watcher;
ev_timer timeout_watcher;

void stdin_cb(EV_P_ ev_io *w, int revents)
{
  (void) revents;
  puts("stdin ready");
  ev_io_stop(EV_A_ w);
  ev_break(EV_A_ EVBREAK_ALL);
}

void timeout_cb(EV_P_ ev_timer *w, int revents)
{
  (void) revents;
  (void) w;
  puts("timeout");
  ev_break(EV_A_ EVBREAK_ONE);
}

int main(void)
{
  struct ev_loop *loop = EV_DEFAULT;
  ev_io_init(&stdin_watcher, stdin_cb, 0, EV_READ);
  ev_io_start(loop, &stdin_watcher);
  ev_timer_init(&timeout_watcher, timeout_cb, 5.5, 0);
  ev_timer_start(loop, &timeout_watcher);
  ev_run(loop, 0); 
  return EXIT_SUCCESS;
}

