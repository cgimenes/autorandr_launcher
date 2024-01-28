CC=clang
LIBS=-lxcb -lxcb-randr
CFLAGS=-Wall -Wextra -O3

autorandr_launcher: autorandr_launcher.c
	$(CC) $(CFLAGS) $(LIBS) $< -o $@

