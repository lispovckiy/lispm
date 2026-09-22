# lispm (Lispovckiy Window Manager)

`lispm` is an ultra-lightweight dynamic binary space partitioning (BSP) window manager for X11 written in C. It uses a client-server architecture with an external IPC tool for window management, keeping the core window manager independent of keybindings.

## Features

* **Separation of Concerns:** Does not handle keyboard input. All window management tasks are offloaded to an external utility.
* **X11-Native IPC:** Communication between the controller and the WM is handled via native X11 ClientMessage events and Atoms, avoiding overhead from sockets or DBus.
* **Dynamic BSP Layout:** Windows are managed in a binary tree structure with on-the-fly split ratio calculations.

## Architecture

* `lispm` — The core window manager (server) that maintains the BSP tree layout.
* `lispmc` — The CLI controller (client) used to send window management commands via X11 events.

## Installation

### Dependencies

* `libx11` (headers and library)

### Build and Install

```bash
git clone https://github.com/lispovckiy/lispm.git
cd lispm
sudo make clean install
```

## Usage and Keybindings

Configuration should be managed via an external hotkey daemon such as `sxhkd`.

### IPC Commands

* Switch workspace: `lispmc view <workspace_index>`
* Move window to workspace: `lispmc move <workspace_index>`
* Resize layout: `lispmc resize <float_delta>`
* Close focused window: `lispmc close`

### Configuration Example (`sxhkdrc`)

```text
# Switch workspaces
super + {1,2}
    lispmc view {0,1}

# Move focused window to workspace
super + shift + {1,2}
    lispmc move {0,1}

# Resize layout horizontally
super + mod1 + {h,l}
    lispmc resize {-0.05,+0.05}


super + f
    lispmc fullscreen

# Close focused window
super + q
    lispmc close
```

