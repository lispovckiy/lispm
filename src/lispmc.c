#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
    if (argc < 2) return 1; 
    Display *display = XOpenDisplay(NULL);
    if (!display) return 1;

    Window root = DefaultRootWindow(display);
    XEvent ev;
    memset(&ev, 0, sizeof(ev)); 

    ev.xclient.type = ClientMessage;
    ev.xclient.window = root;
    ev.xclient.format = 32;

    if (strcmp(argv[1], "close") == 0) {
        ev.xclient.message_type = XInternAtom(display, "CLOSE_WINDOW", False);
    } else if (argc >= 3 && strcmp(argv[1], "view") == 0) {
        ev.xclient.data.l[0] = atol(argv[2]);
        ev.xclient.message_type = XInternAtom(display, "CHANGE_WORKSPACE", False);
    } else if (argc >= 3 && strcmp(argv[1], "move") == 0) {
        ev.xclient.data.l[0] = atol(argv[2]);
        ev.xclient.message_type = XInternAtom(display, "MOVE_WINDOW", False);
    } else {
        XCloseDisplay(display);
        return 1;
    }

    XSendEvent(display, root, False, SubstructureNotifyMask, &ev);
    XFlush(display);
    XCloseDisplay(display);
    return 0;
}
