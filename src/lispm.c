#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include "bsp.h" /* Manage Window */
#include "handle.h" /* Client Messages */



/* Global Varibales */
Display *display; /* Struct Display */
Window Window_root; /* Current Window */
int screen; /* Current Display */
int sw, sh; /* Screen Width & Screen Height */
Atom change_ws_atom;
Atom move_win_atom;
Atom close_window_atom;
Atom resize_window_atom;
Atom net_wm_fullscreen;
Atom floating_atom;
Atom net_current_desktop;

Node *workspaces[WORKSPACES] = {NULL};
int current_workspace = 0;
int gappx = 10; /* Window Gaps */

/* Error Event Function */
int xerror(Display *d, XErrorEvent *ee) {
    (void)d;
    (void)ee;
    return 0;
}

/* check for opening errors */
void DisplayIsOpen(void) {
    if (!(display = XOpenDisplay(NULL))) {
        fprintf(stderr, "lispm: cannot open display\n");
        exit(1);
    } else {
        printf("lispm: window manager started successfully\n");
    }
}

/*
 * Default Settings For Screen
 * screen = DefaultScreen(display) - Set the screen value to the DISPLAY value.
 * sw = DisplayWidth & Height Store the screen width and height in the variables sw and tw.
 *
*/
static void
DefaultSettings(void) {
    screen = DefaultScreen(display);
    Window_root = RootWindow(display, screen);
    sw = DisplayWidth(display, screen);
    sh = DisplayHeight(display, screen);
}

static void
EWMH(void)
{    Window wm_check = XCreateSimpleWindow(display, Window_root, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(display, wm_check, XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False), XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wm_check, 1);
    XChangeProperty(display, wm_check, XInternAtom(display, "_NET_WM_NAME", False), XInternAtom(display, "UTF8_STRING", False), 8, PropModeReplace, (unsigned char *)"lispm", 5);
    XChangeProperty(display, Window_root, XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False), XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wm_check, 1);
}



/* Entry Point */
int
main(void)
{
    XEvent ev; /* Connect Events */

    DisplayIsOpen(); /* call */
    DefaultSettings();
    XSetErrorHandler(xerror); /* call */
    XStoreName(display, DefaultRootWindow(display), "lispm");
    EWMH();

    change_ws_atom = XInternAtom(display, "CHANGE_WORKSPACE", False);
    move_win_atom = XInternAtom(display, "MOVE_WINDOW", False);
    close_window_atom = XInternAtom(display, "CLOSE_WINDOW", False);
    resize_window_atom = XInternAtom(display, "RESIZE_WINDOW", False);
    net_wm_fullscreen = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    floating_atom     = XInternAtom(display, "TOGGLE_FLOATING",          False);
    Atom net_number_of_desktops = XInternAtom(display, "_NET_NUMBER_OF_DESKTOPS", False);
    net_current_desktop = XInternAtom(display, "_NET_CURRENT_DESKTOP", False);

    void handle_client_message(XClientMessageEvent *cme);

    long num_workspaces = 9;
    XChangeProperty(display, Window_root, net_number_of_desktops, XA_CARDINAL, 32,
                PropModeReplace, (unsigned char *)&num_workspaces, 1);

    XSelectInput(display, Window_root, SubstructureRedirectMask | SubstructureNotifyMask);

    while (!XNextEvent(display, &ev)) {
        switch (ev.type) {
            case ClientMessage: {
                handle_client_message(&ev.xclient);
                break;
            }
            case MapRequest: {
                Window w = ev.xmaprequest.window;

                XWindowAttributes wa;
                XGetWindowAttributes(display, w, &wa);
                if (wa.override_redirect) break;

                Atom actual_type;
                int actual_format;
                unsigned long nitems, bytes_after;
                unsigned char *prop = NULL;
                Atom net_wm_window_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
                Atom net_wm_window_type_dock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
               if (XGetWindowProperty(display, w, net_wm_window_type, 0, sizeof(Atom), False,
                                       XA_ATOM, &actual_type, &actual_format, &nitems, &bytes_after, &prop) == Success && prop) {
                    Atom type = *(Atom *)prop;
                    XFree(prop);

                    if (type == net_wm_window_type_dock) {
                        XMapWindow(display, w);
                        break;
                    }
                }


                insert_window(w);
                XMapWindow(display, w);

		XSelectInput(display, w, EnterWindowMask);
		XSetInputFocus(display, w, RevertToParent, CurrentTime);

                arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
                XSync(display, False);
                break;
            }

	    case EnterNotify: {
		if (ev.xcrossing.mode != NotifyNormal || ev.xcrossing.detail == NotifyInferior)
		    break;
	        XSetInputFocus(display, ev.xcrossing.window, RevertToParent, CurrentTime);
                break;
	    }

            case DestroyNotify: {
                Window w = ev.xdestroywindow.window;
                remove_window(w);
                arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
                XSync(display, False);
                break;
            }
            case UnmapNotify: {
                Window w = ev.xunmap.window;
                remove_window(w);
                arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
                XSync(display, False);
                break;
            }
        }
    }

    XCloseDisplay(display);
    return 0;
}

