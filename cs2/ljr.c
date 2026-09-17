#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <linux/uinput.h>

void emit(int fd, int type, int code, int val) {
    struct input_event ie;
    memset(&ie, 0, sizeof(ie));
    ie.type = type;
    ie.code = code;
    ie.value = val;
    if (write(fd, &ie, sizeof(ie)) < 0) return;
}

int main(int argc, char *argv[]) {
    if (argc < 2) return 1;
    int mouse = open(argv[1], O_RDONLY);
    if (mouse < 0) return 1;
    int uifd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (uifd < 0) return 1;

    ioctl(uifd, UI_SET_EVBIT, EV_KEY);
    ioctl(uifd, UI_SET_KEYBIT, KEY_SPACE);
    ioctl(uifd, UI_SET_KEYBIT, KEY_LEFTCTRL);
    ioctl(uifd, UI_SET_KEYBIT, KEY_RIGHTCTRL);
    ioctl(uifd, UI_SET_KEYBIT, KEY_D);
    ioctl(uifd, UI_SET_KEYBIT, KEY_9);

    struct uinput_setup usetup;
    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;
    usetup.id.product = 0x5678;
    strcpy(usetup.name, "ljr");
    ioctl(uifd, UI_DEV_SETUP, &usetup);
    ioctl(uifd, UI_DEV_CREATE);

    struct input_event ie;
    while (read(mouse, &ie, sizeof(ie)) > 0) {
        if (ie.type == EV_KEY && ie.code == 275) {
            if (ie.value == 1) {
                emit(uifd, EV_KEY, KEY_SPACE, 1);
                emit(uifd, EV_KEY, KEY_LEFTCTRL, 1);
                emit(uifd, EV_KEY, KEY_RIGHTCTRL, 1);
                emit(uifd, EV_KEY, KEY_D, 1);
                emit(uifd, EV_KEY, KEY_9, 1);
                emit(uifd, EV_SYN, SYN_REPORT, 0);
                
                emit(uifd, EV_KEY, KEY_9, 0);
                emit(uifd, EV_SYN, SYN_REPORT, 0);
            } else if (ie.value == 0) {
                emit(uifd, EV_KEY, KEY_SPACE, 0);
                emit(uifd, EV_KEY, KEY_LEFTCTRL, 0);
                emit(uifd, EV_KEY, KEY_RIGHTCTRL, 0);
                emit(uifd, EV_KEY, KEY_D, 0);
                emit(uifd, EV_SYN, SYN_REPORT, 0);
            }
        }
    }

    ioctl(uifd, UI_DEV_DESTROY);
    close(uifd);
    close(mouse);
    return 0;
}
