#include <X11/Xlib.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
typedef struct {
    const char* name;
    int min_arguments;
    const char *atom_name;
    void (*handler)(XEvent *ev, int argc, char **argv);
} Commands;

static void
handle_view_move(XEvent *ev, int argc, char **argv);
static void
handle_resize(XEvent *ev, int argc, char **argv);
static void
no_arguments(void);
Commands cmds[] = {
    { "close",      2, "CLOSE_WINDOW",             NULL             },
    { "fullscreen", 2, "_NET_WM_STATE_FULLSCREEN",  NULL            },
    { "view",       3, "CHANGE_WORKSPACE",         handle_view_move },
    { "move",       3, "MOVE_WINDOW",              handle_view_move },
    { "resize",     3, "RESIZE_WINDOW",            handle_resize    },
    { "float",      2, "TOGGLE_FLOATING",          NULL             }
};

static Display*
OpenDisplay(void)
{
    Display *dis = XOpenDisplay(NULL);
    if (!dis) {
        fprintf(stderr, "lispmc: Cannot Open Display");
        exit(1);
    }

    return dis;
}

int main(int argc, char *argv[]) {
    if (argc < 2) { no_arguments(); return 1; }
    Display *dis = OpenDisplay();
    Window root = DefaultRootWindow(dis);
    int num_cmds = sizeof(cmds) / sizeof(cmds[0]);
    bool found = false;

    for (int i = 0; i < num_cmds; i++) {
        if (strcmp(argv[1], cmds[i].name) == 0 ){
            found  = true;

            if (argc < cmds[i].min_arguments) { XCloseDisplay(dis); exit(1);}

            XEvent ev;
            memset(&ev, 0, sizeof(ev));
            ev.xclient.type = ClientMessage;
            ev.xclient.window = root;
            ev.xclient.format = 32;
            ev.xclient.message_type = XInternAtom(dis, cmds[i].atom_name, False);

            if (cmds[i].handler != NULL) {
                cmds[i].handler(&ev, argc, argv);
            }

            XSendEvent(dis, root, False, SubstructureNotifyMask, &ev);
            XFlush(dis);
            break;
        }
    }

    if (!found) {
        no_arguments();
        XCloseDisplay(dis);
        return 1;
    }

    XCloseDisplay(dis);
    return 0;
}


void handle_view_move(XEvent *ev, int argc, char **argv) {
    ev->xclient.data.l[0] = atol(argv[2]);
}

void handle_resize(XEvent *ev, int argc, char **argv) {
    float delta = atof(argv[2]);
    ev->xclient.data.l[0] = (long)(delta * 100);
}
static void
no_arguments(void)
{
    fprintf(stderr, "lispmc: no or invalid arguments\n");
    fprintf(stderr, "available commands: \n");

    int count = sizeof(cmds) / sizeof(cmds[0]);

    for (int i = 0; i < count; i++)
    {
        fprintf(stderr, " %d %s\n", i + 1, cmds[i].name);
    }
}
