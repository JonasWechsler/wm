#ifndef __INFO_H__
#define __INFO_H__

#include <xcb/xcb.h>

struct Info {
  static xcb_connection_t *connection;
  static xcb_drawable_t root;
  static xcb_screen_t *screen;
};

enum DragAction { MOVE, RESIZE, NONE };

struct DragInfo {
  static xcb_window_t window;
  static DragAction action;
};

#endif
