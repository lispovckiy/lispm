#include "handle.h"
#include <X11/Xatom.h>


void
handle_client_message(XClientMessageEvent *cme) {
    Atom move_float_atom;
    move_float_atom = XInternAtom(display, "MOVE_FLOATING_WINDOW", False);
    if (cme->message_type == change_ws_atom) {
        int target = cme->data.l[0];
        view_workspace(target);
        arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);

        long current_ws_long = (long)target;
        XChangeProperty(display, Window_root, net_current_desktop, XA_CARDINAL, 32,
        PropModeReplace, (unsigned char *)&current_ws_long, 1);

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
    } else if (cme->message_type == net_wm_fullscreen) {
        Window focus;
        int revert;
        XGetInputFocus(display, &focus, &revert);

        if (focus != None && focus != Window_root) {
            Node *leaf = find_node_by_win(workspaces[current_workspace], focus);
            if (leaf) {
                leaf->is_fullscreen = !leaf->is_fullscreen;

                if (leaf->is_fullscreen) {
                    XMoveResizeWindow(display, focus, 0, 0, sw, sh);
                    XRaiseWindow(display, focus);
                } else {
                    arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
                }
                XSync(display, False);
            }
            }
        } else if (cme->message_type == floating_atom) {
            Window focus;
            int revert;
            XGetInputFocus(display, &focus, &revert);
            if (focus != None && focus != Window_root) {
                Node *leaf = find_node_by_win(workspaces[current_workspace], focus);
                if (leaf) {
                    leaf->is_floating = !leaf->is_floating;

                    if (leaf->is_floating) {
                        int fw = 800, fh = 600;
                        int fx = (sw - fw) / 2, fy = (sh -fh) / 2;

                        XMoveResizeWindow(display, focus, fx, fy, fw, fh);
                        XRaiseWindow(display, focus);
                    } else {
                        arrange_bsp(workspaces[current_workspace], 0, 0, sw, sh);
                    }
                    XSync(display, False);
                }
            }
        } else if (cme->message_type == move_float_atom) {
            Window focus;
            int revert;
            XGetInputFocus(display, &focus, &revert);
            if (focus != None && focus != Window_root) {
                Node *leaf = find_node_by_win(workspaces[current_workspace], focus);
                if (leaf && leaf->is_floating) {
                    int delta_x = cme->data.l[0], delta_y = cme->data.l[1];
                    leaf->x += delta_x;
                    leaf->y += delta_y;

                    XMoveWindow(display, focus, leaf->x, leaf->y);
                    XSync(display, False);
                }
            }
        }
}

