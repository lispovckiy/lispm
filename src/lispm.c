#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include "bsp.h" /* Manage Window */



/* Global Varibales */
Display *display; /* Struct Display */
Window Window_root; /* Current Window */
int screen; /* Current Display */
int sw, sh; /* Screen Width & Screen Height */
Atom change_ws_atom;
Atom move_win_atom;
Atom close_window_atom;
Atom resize_window_atom;



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
void DefaultSettings(void) {
    screen = DefaultScreen(display);
    Window_root = RootWindow(display, screen);
    sw = DisplayWidth(display, screen);
    sh = DisplayHeight(display, screen);
}

void
handle_client_message(XClientMessageEvent *cme) {
    if (cme->message_type == change_ws_atom) {
        int target = cme->data.l[0];
        view_workspace(target);
        arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
        XSync(display, False);

    } else if (cme->message_type == close_window_atom) {
        Window focus;
        int revert_to;
        XGetInputFocus(display, &focus, &revert_to);

        if (focus != None && focus != Window_root) {
            Atom wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
            Atom wm_delete_window = XInternAtom(display, "WM_DELETE_WINDOW", False);
            Atom *protocols = NULL;
            int n = 0;
            int supports_delete = 0;

            if (XGetWMProtocols(display, focus, &protocols, &n)) {
                while (n--) {
                    if (protocols[n] == wm_delete_window) {
                        supports_delete = 1;
                        break;
                    }
                }
                if (protocols) {
                    XFree(protocols);
                }
            }

            if (supports_delete) {
                XEvent ev;
                ev.type = ClientMessage;
                ev.xclient.window = focus;
                ev.xclient.message_type = wm_protocols;
                ev.xclient.format = 32;
                ev.xclient.data.l[0] = wm_delete_window;
                ev.xclient.data.l[1] = CurrentTime;


                XSendEvent(display, focus, False, NoEventMask, &ev);
            } else {
                XKillClient(display, focus);
            }

            XFlush(display);
            remove_window(focus);


            if (workspaces[current_workspace]) {
                arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
            } else {
                XClearArea(display, Window_root, 0, 0, sw, sh, False);
            }
        }

    } else if (cme->message_type == resize_window_atom) {
        Window focus;
        int revert_to;
        XGetInputFocus(display, &focus, &revert_to);
        if (focus != None && focus != Window_root) {
            Node *leaf = find_node_by_win(workspaces[current_workspace], focus);
            if (leaf && leaf->parent) {
                float delta = (float)cme->data.l[0] / 100.0f;
                leaf->parent->split_ratio += delta;
                arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
            }
        }
    } else if (cme->message_type == move_win_atom) {
        int target = cme->data.l[0];
        Window focus;
        int revert;
        XGetInputFocus(display, &focus, &revert);

        if (focus != None && focus != Window_root) {
            move_window_to_workspace(focus, target);
            arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
            XSync(display, False);
        }
    }
}


void EWMH(void) {
    Window wm_check = XCreateSimpleWindow(display, Window_root, 0, 0, 1, 1, 0, 0, 0);
    XChangeProperty(display, wm_check, XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False), XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wm_check, 1);
    XChangeProperty(display, wm_check, XInternAtom(display, "_NET_WM_NAME", False), XInternAtom(display, "UTF8_STRING", False), 8, PropModeReplace, (unsigned char *)"lispm", 5);
    XChangeProperty(display, Window_root, XInternAtom(display, "_NET_SUPPORTING_WM_CHECK", False), XA_WINDOW, 32, PropModeReplace, (unsigned char *)&wm_check, 1);
}



/* Entry Point */

int main(void) {
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

