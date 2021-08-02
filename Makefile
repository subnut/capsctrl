.POSIX:
.SUFFIXES:

__CFLAGS__	= -Wall ${CFLAGS}
__CPPFLAGS__	= ${CPPFLAGS}
__LDFLAGS__	= ${LDFLAGS}

all: caps_hold_ctrl
clean: ; rm -f caps_hold_ctrl

caps_hold_ctrl: caps_hold_ctrl.c
	@${MAKE} _build CPPFLAGS="${__CPPFLAGS__}" \
		CFLAGS="`pkg-config --cflags libevdev` ${__CFLAGS__}" \
		LDFLAGS="`pkg-config --libs libevdev` ${__LDFLAGS__}"

_build: caps_hold_ctrl.c
	"${CC}" ${CFLAGS} ${CPPFLAGS} ${LDFLAGS} caps_hold_ctrl.c -o caps_hold_ctrl
