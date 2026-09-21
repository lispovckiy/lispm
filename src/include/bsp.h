#ifndef BSP_H
#define BSP_H

#include <X11/Xlib.h>

#define WORKSPACES 9

typedef struct Node {
    Window win;
    int x, y, w, h;
    struct Node *left;
    struct Node *right;
    struct Node *parent;
} Node;

extern int sw, sh;
extern Display *display;
extern int gappx;
extern Node *workspaces[WORKSPACES];
extern int current_workspace;


void arrange_bsp(Node *node, int x, int y, int w, int h);
void insert_window(Window w);
void remove_window(Window w);
void view_workspace(int ws);
void move_window_to_workspace(Window w, int workspace_index); 
void hide_node(Node *node);
void show_node(Node *node);



#endif

