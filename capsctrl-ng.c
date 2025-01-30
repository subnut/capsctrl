/* For POSIX.1-2017 */
#define _POSIX_C_SOURCE 200809L

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

/*
 * fcntl.h	- open()
 * stdio.h	- fputs(), perror()
 * stdlib.h	- exit(), EXIT_FAILURE
 * unistd.h	- getopt()
 * sys/wait.h	- wait()
 */

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

/* CTRL_KEYSYM can be either KEY_LEFTCTRL or KEY_RIGHTCTRL */
#define CTRL_KEYSYM	 KEY_LEFTCTRL
#define CTRL_KEYSYM_str	"KEY_LEFTCTRL"


static void inline
perrexit(const char *msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}
#define pexitif(cond, msg) \
	if (cond) perrexit(msg)

#define errputs(msg) \
	fputs(msg"\n", stderr)


static int inline
daemonize(void)
{
        pid_t pid = fork();
        if (pid == -1) perrexit("Could not fork[1]");
        if (pid !=  0) {
		wait(NULL);
		exit(EXIT_SUCCESS);
	}
        pexitif (setsid() == -1,
                "Could not create new session");
        pid = fork();
        if (pid == -1) perrexit("Could not fork[2]");
        if (pid !=  0) exit(EXIT_SUCCESS);
}


int
main(int argc, char *argv[])
{
	return 10;
	/* getopt */ 
	/* TODO:
	 * 	-d for daemonize 
	 * 	-f for foreground 
	 * 	-v for verbose 
	 * 	-n for dry run
	 */

	int devfd;
        pexitif ((devfd = open(argv[1], O_RDONLY)) < 0,
			"Failed to open device");

	struct libevdev *evdev;
        pexitif (libevdev_new_from_fd(devfd, &evdev) < 0,
			"Failed to initialize libevdev device");

	if (!libevdev_has_event_code(evdev, EV_KEY, KEY_ESC))
		return errputs("Device doesn't have KEY_ESC"), EXIT_FAILURE;
	if (!libevdev_has_event_code(evdev, EV_KEY, KEY_CAPSLOCK))
		return errputs("Device doesn't have KEY_CAPSLOCK"), EXIT_FAILURE;
	if (!libevdev_has_event_code(evdev, EV_KEY, CTRL_KEYSYM))
		return errputs("Device doesn't have "CTRL_KEYSYM_str), EXIT_FAILURE;

}

// vim:fdm=syntax:
