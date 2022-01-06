/* For POSIX.1-2017 */
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * errno.h  -   errno, perror()
 * fcntl.h  -   open(), O_RDONLY
 * time.h   -   clock_gettime()
 * stdio.h  -   fputs()
 * stdlib.h -   EXIT_{SUCCESS,FAILURE}, exit()
 * string.h -   strcmp(), strncmp()
 * unistd.h -   close()
 */

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

/* CTRL_KEYSYM can be either KEY_LEFTCTRL or KEY_RIGHTCTRL */
#define CTRL_KEYSYM KEY_LEFTCTRL
#define DELAY 300 // In milliseconds


#if     defined(CLOCK_MONOTONIC_RAW)
#define CLOCK   CLOCK_MONOTONIC_RAW
#elif   defined(CLOCK_MONOTONIC)
#define CLOCK   CLOCK_MONOTONIC
#else        // CLOCK_REALTIME is always available in POSIX-1.2017
#define CLOCK   CLOCK_REALTIME
#endif


/* Returns current time in microseconds */
static inline long int
gettime(void)
{
        struct timespec timespec;
        if (clock_gettime(CLOCK, &timespec) != 0)
        {
                perror("FATAL: clock_gettime error"),
                exit(EXIT_FAILURE);
        }
        return ((timespec.tv_sec * 1000L) + (timespec.tv_nsec / 1000000L));
        /* tv_nsec is long int */
}

static inline int
uinput_write_event(struct libevdev_uinput *uinput_dev, struct input_event *event)
{
#ifdef EBUG
        fprintf(stderr, "SENT: %s %s %d\n",
                libevdev_event_type_get_name(event->type),
                libevdev_event_code_get_name(event->type, event->code),
                event->value);
#endif
        return libevdev_uinput_write_event(uinput_dev,
                        event->type, event->code, event->value);
}

static inline int
next_event(struct libevdev *dev, struct input_event *event)
{
        int retcode = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, event);
#ifdef EBUG
        if (retcode == 0)
        {
                fprintf(stderr, "GOT: %s %s %d\n",
                                libevdev_event_type_get_name(event->type),
                                libevdev_event_code_get_name(event->type, event->code),
                                event->value
                       );
        }
#endif
        return retcode;
}

int
main(int argc, char *argv[])
{
        if (argc != 2 || !strcmp(argv[1], "-h") || !strncmp(argv[1], "--h", 3))
                return fprintf(stderr,
                                "Usage: %s <device>\n"
                                "\n"
                                "\t<device>\tPath to device inside /dev/input\n"
                                "\n"
                                , argv[0]),
                       (argc != 2 ? EXIT_FAILURE : EXIT_SUCCESS);

        int retcode;
        int dev_fd;
        struct libevdev         *dev;
        struct libevdev_uinput  *uinput_dev;
        struct input_event      event;

        struct {
                long int since;
                enum { UP, DOWN, CTRL } state;
        } CapsState;

        /* We're assuming that CapsLock isn't already held down */
        CapsState.state = UP;
        CapsState.since = gettime();

        /*
         * We don't use O_NONBLOCK, since that causes the loop to keep running
         * as fast as possible, but we want it to run only when key is pressed
         */
        if ((dev_fd = open(argv[1], O_RDONLY)) < 0)
                return perror("Failed to open given input"),
                       EXIT_FAILURE;

        if ((dev = libevdev_new()) == NULL)
                return perror("Failed to allocate memory for device"),
                       EXIT_FAILURE;

        if ((retcode = libevdev_set_fd(dev, dev_fd)) < 0)
                return errno = -retcode,
                       perror("Failed to initialize libevdev device"),
                       EXIT_FAILURE;

        if (!libevdev_has_event_code(dev, EV_KEY, KEY_CAPSLOCK)
                        || !libevdev_has_event_code(dev, EV_KEY, KEY_ESC)
                        || !libevdev_has_event_code(dev, EV_KEY, KEY_LEFTCTRL))
                return fputs("Specified input doesn't have CAPSLOCK/ESC/LEFTCTRL\n", stderr),
                       EXIT_FAILURE;

        if ((retcode = libevdev_grab(dev, LIBEVDEV_GRAB)) < 0)
                return errno = -retcode,
                       perror("Failed to grab device"),
                       EXIT_FAILURE;

        if ((retcode = libevdev_uinput_create_from_device(dev,
                                        LIBEVDEV_UINPUT_OPEN_MANAGED,
                                        &uinput_dev)) != 0)
                return errno = -retcode,
                       perror("Failed to open uinput"),
                       EXIT_FAILURE;

        for (retcode = next_event(dev, &event);
                        retcode == LIBEVDEV_READ_STATUS_SUCCESS;
                        retcode = next_event(dev, &event))
        {
                switch (event.code)
                {
                        case KEY_RIGHTALT: event.code = KEY_RIGHTCTRL; break;
                        case KEY_RIGHTCTRL: event.code = KEY_RIGHTALT; break;
                        default: break;
                }

                if (event.code == KEY_CAPSLOCK && event.value == 2)
                {
                        CapsState.state = CTRL;
                        event.code = CTRL_KEYSYM;
                        event.value = 1;
                }

                if (event.code == KEY_CAPSLOCK)
                {
                        if (event.value == 1)
                        {
                                CapsState.state = DOWN;
                                CapsState.since = gettime();
                                /*
                                 * Don't write this event to uinput, 'cause we
                                 * aren't if this is a CapsLock or a Control
                                 */
                                continue;
                        }
                        else /* event.value == 0 */
                        {
                                if (CapsState.state == CTRL)
                                {
                                        CapsState.state = UP;
                                        event.code = CTRL_KEYSYM;
                                }
                                else /* CapsState.state == DOWN */
                                {
                                        CapsState.state = UP;
                                        if ((gettime() - CapsState.since) > DELAY)
                                                /*
                                                 * The button was held down for too long.
                                                 * ie. The user was thinking of executing
                                                 * a keychord beginning with CTRL, but
                                                 * never got beyond the CTRL key.
                                                 *
                                                 * This event should be ignored.
                                                 */
                                                continue;

                                        /*
                                         * The interval between pressing CapsLock and
                                         * releasing it is less than the configured delay
                                         *
                                         * The user wants to toggle CapsLock
                                         */
                                        event.value = 1; /* To signify pressing down */
                                        if ((retcode = uinput_write_event(uinput_dev, &event)) < 0)
                                                return errno = -retcode,
                                                       perror("Failed to write event to uinput"),
                                                       EXIT_FAILURE;
                                        event.value = 0; /* To signify releasing it */
                                }

                        }
                }
                else if (event.type == EV_KEY)
                {
                        if (CapsState.state == DOWN)
                        {
                                /* User has pressed a different key while holding down CapsLock */
                                CapsState.state = CTRL;

                                struct input_event event_copy;
                                event_copy.code =  event.code;
                                event_copy.value =  event.value;

                                event.code = CTRL_KEYSYM;
                                event.value = 1;
                                if ((retcode = uinput_write_event(uinput_dev, &event)) < 0)
                                        return errno = -retcode,
                                               perror("Failed to write event to uinput"),
                                               EXIT_FAILURE;

                                event.code =  event_copy.code;
                                event.value =  event_copy.value;
                        }

                }

                if ((retcode = uinput_write_event(uinput_dev, &event)) < 0)
                        return errno = -retcode,
                               perror("Failed to write event to uinput"),
                               EXIT_FAILURE;
        }

        switch (retcode)
        {
                case -ENODEV:
                        fprintf(stderr, "Device disconnected: %s\n", argv[1]);
                        break;

                case LIBEVDEV_READ_STATUS_SYNC:
                        fputs("WARNING: libevdev_next_event returned "
                                        "LIBEVDEV_READ_STATUS_SYNC\n",
                                        stderr);
                        break;

                case -EAGAIN:
                        fputs("WARNING: libevdev_next_event returned "
                                        "EAGAIN\n",
                                        stderr);
                        break;

                default:
                        errno = -retcode;
                        return perror("FATAL: libevdev_next_event error"),
                               EXIT_FAILURE;
        }

        libevdev_uinput_destroy(uinput_dev);
        libevdev_free(dev);
        close(dev_fd);
        return EXIT_SUCCESS;
}

// vim:set et ts=8 sw=8 sts=4 nowrap path+=/usr/include/* fdm=syntax:
