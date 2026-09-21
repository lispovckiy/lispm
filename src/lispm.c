#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include "bsp.h" /* Manage Window */

/* Global Varibales */
Display *display; /* Struct Display */
Window Window_root; /* Current Window */
int screen; /* Current Display */
int sw, sh; /* Screen Width & Screen Height */

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


/* Entry Point */

int main(void) {
    XEvent ev; /* Connect Events */
    Atom change_ws_atom;
    Atom move_win_atom;

    DisplayIsOpen(); /* call */
    DefaultSettings();
    XSetErrorHandler(xerror); /* call */
    XStoreName(display, DefaultRootWindow(display), "lispm");

    change_ws_atom = XInternAtom(display, "CHANGE_WORKSPACE", False);
    move_win_atom = XInternAtom(display, "MOVE_WINDOW", False);
    Atom close_window_atom = XInternAtom(display, "CLOSE_WINDOW", False);
    Atom resize_window_atom = XInternAtom(display, "RESIZE_WINDOW", False);


    XSelectInput(display, Window_root, SubstructureRedirectMask | SubstructureNotifyMask);

    while (!XNextEvent(display, &ev)) {
        switch (ev.type) {
	    case ClientMessage: {
		if (ev.xclient.message_type == change_ws_atom) {
		   int target = ev.xclient.data.l[0];
		   view_workspace(target);
		   arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
		   XSync(display, False);

		}
                if (ev.xclient.message_type == close_window_atom) {
			Window focus;
       		        int revert_to;
			XGetInputFocus(display, &focus, &revert_to);
			
			if (focus != None && focus != DefaultRootWindow(display)) {
		            remove_window(Window_root);
			    XKillClient(display, focus); 
			    if (workspaces[current_workspace]) {
				 arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
                            } else {
				XClearArea(display, Window_root, 0, 0, sw, sh, False);
			    }
			}

                }

		if (ev.xclient.message_type == resize_window_atom) {
		    Window focus;
		    int revert_to;
		    XGetInputFocus(display, &focus, &revert_to);
		    if (focus != None && focus != Window_root) {
			Node *leaf = find_node_by_win(workspaces[current_workspace], focus);
			if (leaf && leaf->parent) {
			   float delta = (float)ev.xclient.data.l[0] / 100.0f;
			   leaf->parent->split_ratio += delta;
			   arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
			}
		    }

		}

		else if (ev.xclient.message_type == move_win_atom) {
		    int target = ev.xclient.data.l[0];
		    Window focus;
		    int revert;
		    XGetInputFocus(display, &focus, &revert);

		    if (focus != None && focus != Window_root) {
			move_window_to_workspace(focus, target);
			arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
                        XSync(display, False);
		    }
		}
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

