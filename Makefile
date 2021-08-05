.POSIX:
CFLAGS = `pkg-config --cflags libevdev` -O2 -Wall
LDFLAGS = `pkg-config --libs libevdev`
all: capsctrl
clean: ; rm -f capsctrl
capsctrl: capsctrl.c
