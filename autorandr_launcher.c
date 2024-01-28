#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <xcb/randr.h>

static int launch_autorandr(void) {
  pid_t pid = fork();
  if (pid == 0) {
    if (execlp("autorandr", "--change --default default") == -1) {
      int errsv = errno;
      fprintf(stderr, "Error executing file: %s\n", strerror(errsv));
      exit(errsv);
    }

    exit(127);
  } else {
    waitpid(pid, 0, 0);
  }
  return 0;
}

int main(int argc, char **argv) {
  int help = 0;

  const struct option long_options[] = {
      {"help", no_argument, &help, 1},
  };
  static const char *short_options = "h";

  const char *help_str =
      "Usage: autorandr_launcher [OPTION]\n"
      "\n"
      "Listens to X server screen change events and launches autorandr after "
      "an event occurs.\n"
      "\n"
      "\t-h,--help\t\t\tDisplay this help and exit\n";

  int option_index = 0;
  int ch = 0;
  while (ch != -1) {
    ch = getopt_long(argc, argv, short_options, long_options, &option_index);
    switch (ch) {
    case 'h':
      help = 1;
      break;
    }
  }

  if (help == 1) {
    printf("%s", help_str);
    exit(0);
  }

  // Check for already running daemon?
  // Check if autorandr is in PATH
  // Create service and/or change the sleep.target one
  // Add to AUR

  int screenNum;
  xcb_connection_t *c = xcb_connect(NULL, &screenNum);
  int conn_error = xcb_connection_has_error(c);
  if (conn_error) {
    fprintf(stderr, "Connection error!\n");
    exit(conn_error);
  }
  // Get the screen whose number is screenNum
  const xcb_setup_t *setup = xcb_get_setup(c);
  xcb_screen_iterator_t iter = xcb_setup_roots_iterator(setup);

  // we want the screen at index screenNum of the iterator
  for (int i = 0; i < screenNum; ++i) {
    xcb_screen_next(&iter);
  }
  xcb_screen_t *default_screen = iter.data;
  printf("Connected to server\n");

  // Subscribe to screen change events
  xcb_randr_select_input(c, default_screen->root,
                         XCB_RANDR_NOTIFY_MASK_SCREEN_CHANGE);
  xcb_flush(c);

  printf("Waiting for event\n");
  xcb_generic_event_t *evt;
  while ((evt = xcb_wait_for_event(c))) {
    printf("Event type: %u\n", evt->response_type);

    if (evt->response_type) {
      printf("Launch autorandr!\n");
      launch_autorandr();
    }
    free(evt);
  }
}
