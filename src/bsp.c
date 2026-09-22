#include <stdlib.h>
#include "bsp.h"


void arrange_bsp(Node *node, int x, int y, int w, int h) {
    if (!node) return;

    if (node->is_fullscreen) {
        XMoveResizeWindow(display, node->win, 0, 0, sw, sh);
        XRaiseWindow(display, node->win);

        node->x = 0; node->y = 0; node->w = sw; node->h = sh;
        return;
    }

    node->x = x; node->y = y; node->w = w; node->h = h;

    if (!node->left && !node->right && node->win != 0) {
        XMoveResizeWindow(display, node->win,
                          x + gappx, y + gappx,
                          w - (2 * gappx), h - (2 * gappx));
        return;
    }

    if (node->left && node->right) {
        if (node->split_ratio <= 0.05f || node->split_ratio >= 0.95f) {
            node->split_ratio = 0.5f;
        }

        if (w > h) {
            int lw = (int)(w * node->split_ratio);
            arrange_bsp(node->left, x, y, lw, h);
            arrange_bsp(node->right, x + lw, y, w - lw, h);
        } else {
            int lh = (int)(h * node->split_ratio);
            arrange_bsp(node->left, x, y, w, lh);
            arrange_bsp(node->right, x, y + lh, w, h - lh);
        }
    }
}

Node* find_leaf(Node *node, int w, int h) {
    if (!node) return NULL;
    if (!node->left && !node->right) return node;
    if (w > h) {
        return find_leaf(node->left, w / 2, h);
    } else {
        return find_leaf(node->left, w, h / 2);
    }
}

void insert_window(Window w) {
    if (!workspaces[current_workspace]) {
        workspaces[current_workspace] = calloc(1, sizeof(Node));
        workspaces[current_workspace]->win = w;
        return;
    }
    Node *leaf = find_leaf(workspaces[current_workspace], sw, sh);
    if (!leaf) return;
    Node *new_left = calloc(1, sizeof(Node));
    Node *new_right = calloc(1, sizeof(Node));
    leaf->split_ratio = 0.5f;
    new_left->win = leaf->win;
    new_left->parent = leaf;
    new_right->win = w;
    new_right->parent = leaf;
    leaf->win = 0;
    leaf->left = new_left;
    leaf->right = new_right;
}


Node* find_node_by_win(Node *node, Window w) {
    if (!node) return NULL;
    if (node->win == w) return node;

    Node *found = find_node_by_win(node->left, w);
    if (found) return found;
    return find_node_by_win(node->right, w);
}

void remove_window(Window w) {
    Node *node = find_node_by_win(workspaces[current_workspace], w);
    if (!node) return;

    Node *parent = node->parent;
    if (!parent) {
        free(workspaces[current_workspace]);
        workspaces[current_workspace] = NULL;
        return;
    }

    Node *sibling = (parent->left == node) ? parent->right : parent->left;

    parent->win = sibling->win;
    parent->left = sibling->left;
    parent->right = sibling->right;

    if (parent->left) parent->left->parent = parent;
    if (parent->right) parent->right->parent = parent;

    free(node);
    free(sibling);
}

void show_node(Node *node) {
    if (!node) return;
    if (node->win != 0) {
        XMapWindow(display, node->win);
    }
    show_node(node->left);
    show_node(node->right);
}

void hide_node(Node *node) {
    if (!node) return;
    if (node->win != 0) {
        XUnmapWindow(display, node->win);
    }
    hide_node(node->left);
    hide_node(node->right);
}


void view_workspace(int ws) {
    if (ws < 0 || ws >= WORKSPACES || ws == current_workspace) return;
    hide_node(workspaces[current_workspace]);
    current_workspace = ws;
    show_node(workspaces[current_workspace]);
}

void move_window_to_workspace(Window w, int ws) {
    if (ws < 0 || ws >= WORKSPACES || ws == current_workspace) return;
    Node *node = find_node_by_win(workspaces[current_workspace], w);
    if (!node) return;
    remove_window(w);
    int backup_ws = current_workspace;
    current_workspace = ws;
    insert_window(w);
    current_workspace = backup_ws;
    XUnmapWindow(display, w);
}


