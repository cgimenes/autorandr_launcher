#include <errno.h>
#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <xcb/randr.h>
#include <xcb/xcb.h>

extern char **environ;

static int ar_launch(void) {
  printf("Launch autorandr!\n");
  static const char *argv[] = {"autorandr", "--change", "--default", "default",
                               NULL};
  char **comm = malloc(sizeof argv);
  memcpy(comm, argv, sizeof argv);

  pid_t pid = fork();
  if (pid == 0) {
    if (execvp(argv[0], comm) == -1) {
      int errsv = errno;
      fprintf(stderr, "Error executing file: %s\n", strerror(errsv));
      exit(errsv);
    }

    exit(127);
  } else {
    waitpid(pid, 0, 0);
    free(comm);
  }
  return 0;
}

int main() {
  xcb_connection_t *conn;
  xcb_screen_t *screen;
  xcb_window_t window;
  xcb_generic_event_t *evt;
  xcb_randr_screen_change_notify_event_t *randr_evt;
  xcb_timestamp_t last_time;

  conn = xcb_connect(NULL, NULL);
  int conn_error = xcb_connection_has_error(conn);
  if (conn_error) {
    fprintf(stderr, "Connection error!\n");
    exit(conn_error);
  }

  screen = xcb_setup_roots_iterator(xcb_get_setup(conn)).data;
  window = screen->root;
  xcb_randr_select_input(conn, window, XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE);
  xcb_flush(conn);

  printf("Listening for events\n");
  while ((evt = xcb_wait_for_event(conn)) != NULL) {
    printf("Event type: %u\n", evt->response_type);
    if (evt->response_type & XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE) {
      randr_evt = (xcb_randr_screen_change_notify_event_t *)evt;
      if (last_time != randr_evt->timestamp) {
        last_time = randr_evt->timestamp;
        ar_launch();
      } else {
        printf("Ignoring event.\n");
      }
    } else {
      printf("Ignoring event: %u\n", evt->response_type);
    }
    free(evt);
  }
  xcb_disconnect(conn);
}
