#ifndef BSP_H
#define BSP_H

#include <X11/Xlib.h>
#include <stdbool.h>

#define WORKSPACES 9

typedef struct Node {
    Window win;
    int x, y, w, h;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
    float split_ratio;
    bool is_fullscreen;
    bool is_floating;
} Node;

extern int sw, sh;
extern Display *display;
extern int gappx;
extern Node *workspaces[WORKSPACES];
extern int current_workspace;
extern Atom change_ws_atom;
extern Atom move_win_atom;
extern Atom close_window_atom;
extern Atom resize_window_atom;
extern Window Window_root;
extern Atom net_wm_fullscreen;
extern Atom floating_atom;
void
handle_client_message(XClientMessageEvent *cme);
void arrange_bsp(Node *node, int x, int y, int w, int h);
void insert_window(Window w);
void remove_window(Window w);
void view_workspace(int ws);
void move_window_to_workspace(Window w, int workspace_index);
void hide_node(Node *node);
void show_node(Node *node);
Node* find_node_by_win(Node *node, Window w);


#endif
