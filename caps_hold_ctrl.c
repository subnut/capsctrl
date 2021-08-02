#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/*
 * errno.h  -   errno, perror()
 * fcntl.h  -   open(), O_RDONLY
 * stdio.h  -   fputs()
 * stdlib.h -   EXIT_{SUCCESS,FAILURE}
 * unistd.h -   close()
 */

#include <libevdev/libevdev.h>
#include <libevdev/libevdev-uinput.h>

int
main(int argc, char *argv[])
{
        if (argc != 2)
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
        struct libevdev_uinput  *uinput_dev;
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

        do {
                retcode = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &event);
                if (retcode == LIBEVDEV_READ_STATUS_SUCCESS)
                        if (libevdev_uinput_write_event(uinput_dev, event.type, event.code, event.value) < 0)
                                return errno = -retcode,
                                       perror("Failed to write event to uinput"),
                                       EXIT_FAILURE;

                switch (retcode)
                {
                        case LIBEVDEV_READ_STATUS_SUCCESS:
                                fprintf(stderr, "Event: %s %s %d\n",
                                                libevdev_event_type_get_name(event.type),
                                                libevdev_event_code_get_name(event.type, event.code),
                                                event.value
                                       );
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
                }
        } while (retcode == LIBEVDEV_READ_STATUS_SUCCESS
                        || retcode == LIBEVDEV_READ_STATUS_SYNC
                        || retcode == -EAGAIN);

        /* XXX: Unreachable code below */
        libevdev_uinput_destroy(uinput_dev);
        libevdev_free(dev);
        close(dev_fd);
        return EXIT_SUCCESS;
}


// vim:set et ts=8 sw=8 sts=4 nowrap path+=/usr/include/* fdm=syntax:
