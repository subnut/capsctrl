.POSIX:
.SUFFIXES:

__CFLAGS__	= -Wall ${CFLAGS}
__CPPFLAGS__	= ${CPPFLAGS}
__LDFLAGS__	= ${LDFLAGS}

all: capsctrl
clean: ; rm -f capsctrl

capsctrl: capsctrl.c
	@${MAKE} _build CPPFLAGS="${__CPPFLAGS__}" \
		CFLAGS="`pkg-config --cflags libevdev` ${__CFLAGS__}" \
		LDFLAGS="`pkg-config --libs libevdev` ${__LDFLAGS__}"

_build: capsctrl.c
	"${CC}" ${CFLAGS} ${CPPFLAGS} ${LDFLAGS} capsctrl.c -o capsctrl
