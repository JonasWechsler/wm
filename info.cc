#include "info.h"

xcb_connection_t *Info::connection = nullptr;
xcb_drawable_t Info::root;
xcb_screen_t *Info::screen = nullptr;

xcb_drawable_t DragInfo::window;
DragAction DragInfo::action;
