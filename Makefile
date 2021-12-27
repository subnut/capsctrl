.POSIX:
all:

CFLAGS          = -Wall -O2
DESTDIR         = /usr/local/bin
EVDEV_CFLAGS    = `pkgconf --cflags libevdev`
EVDEV_LDFLAGS   = `pkgconf --libs   libevdev`

all: build
build: checkdeps capsctrl
clean: ;@if [ -f capsctrl ]; then echo rm -f capsctrl; rm -f capsctrl; fi
	@if [ -f logger   ]; then echo rm -f logger  ; rm -f logger  ; fi

install: build
	@if [ -f "$(DESTDIR)/capsctrl" ]; then rm -rf "$(DESTDIR)/capsctrl"; fi
	cp capsctrl "$(DESTDIR)/capsctrl"
	chown root:root "$(DESTDIR)/capsctrl"
	chmod 4111 "$(DESTDIR)/capsctrl"

uninstall:
	rm -rf "$(DESTDIR)/capsctrl"


checkbuilddeps:
	@command -v pkgconf >/dev/null || {			\
		echo pkgconf not found;				\
		echo Please install pkgconf before building;	\
		exit 1;						\
	}

checkdeps: checkbuilddeps
	@pkgconf libevdev || { 					\
		echo pkgconf "couldn't" find libevdev;		\
		echo Please check if libevdev is installed and	\
		pkgconf is configured properly;			\
		exit 1;						\
	}


CFLAGS_  = $(EVDEV_CFLAGS) $(CFLAGS)
LDFLAGS_ = $(EVDEV_LDFLAGS) $(LDFLAGS)

.SUFFIXES:
.SUFFIXES: .c
.c: ; $(CC) $(CFLAGS_) $(LDFLAGS_) -o $@ $<
