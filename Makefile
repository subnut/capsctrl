.POSIX:

CFLAGS = `pkg-config --cflags libevdev` -O2 -Wall
LDFLAGS = `pkg-config --libs libevdev`

all: checkdeps capsctrl
clean: ; rm -f capsctrl logger
uninstall: ; DESTDIR="$(DESTDIR)"; DESTDIR="$${DESTDIR:-/usr/local/bin}"; rm -rf "$${DESTDIR}/capsctrl"
install: ; DESTDIR="$(DESTDIR)"; DESTDIR="$${DESTDIR:-/usr/local/bin}"; cp capsctrl "$${DESTDIR}/capsctrl"; \
	chown root:root "$${DESTDIR}/capsctrl"; chmod 4111 "$${DESTDIR}/capsctrl"

logger: logger.c
capsctrl: capsctrl.c

checkbuilddeps:
	@if ! command -v pkg-config >/dev/null; then echo pkg-config not found; \
	echo Please install pkgconfig before building; exit 1; fi

checkdeps: checkbuilddeps
	@if ! pkg-config libevdev; then echo pkg-config "couldn't" find libevdev; \
	echo Please check if libevdev is installed and pkg-config is configured \
	properly; exit 1; fi
