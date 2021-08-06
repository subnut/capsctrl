#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

static inline int
next_event(struct libevdev *dev, struct input_event *event)
{
        int retcode = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, event);
        if (retcode == 0)
                fprintf(stderr, "GOT: %s %s %d\n",
                                libevdev_event_type_get_name(event->type),
                                libevdev_event_code_get_name(event->type, event->code),
                                event->value
                       );
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
                       EXIT_FAILURE;

        int retcode;
        int dev_fd;
        struct libevdev         *dev;
        struct input_event      event;

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

        for (retcode = next_event(dev, &event);
                        retcode == LIBEVDEV_READ_STATUS_SUCCESS;
                        retcode = next_event(dev, &event)) {;}

        switch (retcode)
        {
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
                        perror("FATAL: libevdev_next_event error");
        }

        libevdev_free(dev);
        close(dev_fd);
        return EXIT_SUCCESS;
}

// vim:set et ts=8 sw=8 sts=4 nowrap path+=/usr/include/* fdm=syntax:
