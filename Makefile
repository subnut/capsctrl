.POSIX:
CFLAGS = `pkg-config --cflags libevdev` -O2 -Wall
LDFLAGS = `pkg-config --libs libevdev`
all: checkdeps capsctrl
capsctrl: capsctrl.c
logger: logger.c
clean: ; rm -f capsctrl logger
checkbuilddeps:
	@if ! command -v pkg-config >/dev/null; then echo pkg-config not found; \
	echo Please install pkgconfig before building; exit 1; fi
checkdeps: checkbuilddeps
	@if ! pkg-config libevdev; then echo pkg-config "couldn't" find libevdev; \
	echo Please check if libevdev is installed and pkg-config is configured \
	properly; exit 1; fi
