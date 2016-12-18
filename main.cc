#include "info.h"
#include "log.h"
#include <algorithm>
#include <stdint.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <xcb/xcb.h>

void focus(xcb_window_t &window) {
  const uint32_t mode[1] = {XCB_STACK_MODE_ABOVE};
  xcb_configure_window(Info::connection, window, XCB_CONFIG_WINDOW_STACK_MODE,
                       mode);
  xcb_set_input_focus(Info::connection, XCB_INPUT_FOCUS_POINTER_ROOT, window,
                      XCB_CURRENT_TIME);
}

void event(void) {
  log_verbose("waiting...");
  xcb_generic_event_t *event = xcb_wait_for_event(Info::connection);

  log_verbose("event response %d %s", event->response_type & ~0x80,
              (DragInfo::action == MOVE)
                  ? "(move)"
                  : (DragInfo::action == RESIZE) ? "(resize)" : "");

  switch (event->response_type & ~0x80) {

  case XCB_BUTTON_PRESS: {
    log_verbose("XCB_BUTTON_PRESS");

    xcb_button_press_event_t *press_event = (xcb_button_press_event_t *)event;
    DragInfo::window = press_event->child;

    focus(DragInfo::window);

    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(
        Info::connection, xcb_get_geometry(Info::connection, DragInfo::window),
        NULL);

    if (press_event->detail == 1) {
      DragInfo::action = MOVE;
      xcb_warp_pointer(Info::connection, XCB_NONE, DragInfo::window, 0, 0, 0, 0,
                       1, 1);
    } else {
      DragInfo::action = RESIZE;
      xcb_warp_pointer(Info::connection, XCB_NONE, DragInfo::window, 0, 0, 0, 0,
                       geom->width, geom->height);
    }

    xcb_grab_pointer(Info::connection, 0, Info::root,
                     XCB_EVENT_MASK_BUTTON_RELEASE |
                         XCB_EVENT_MASK_BUTTON_MOTION |
                         XCB_EVENT_MASK_POINTER_MOTION_HINT,
                     XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, Info::root,
                     XCB_NONE, XCB_CURRENT_TIME);
  } break;

  case XCB_MOTION_NOTIFY: {
    log_verbose("XCB_MOTION_NOTIFY");

    if (DragInfo::action == NONE)
      break;

    xcb_query_pointer_reply_t *pointer = xcb_query_pointer_reply(
        Info::connection, xcb_query_pointer(Info::connection, Info::root), 0);
    xcb_get_geometry_reply_t *geom = xcb_get_geometry_reply(
        Info::connection, xcb_get_geometry(Info::connection, DragInfo::window),
        NULL);

    if (DragInfo::action == MOVE) { /* move */
      uint32_t min_x = Info::screen->width_in_pixels - geom->width;
      uint32_t min_y = Info::screen->height_in_pixels - geom->height;

      const uint32_t point[2] = {std::min((uint32_t)pointer->root_x, min_x),
                                 std::min((uint32_t)pointer->root_y, min_y)};

      xcb_configure_window(Info::connection, DragInfo::window,
                           XCB_CONFIG_WINDOW_X | XCB_CONFIG_WINDOW_Y, point);
    } else if (DragInfo::action == RESIZE) { /* resize */
      const uint32_t point[2] = {(uint32_t)(pointer->root_x - geom->x),
                                 (uint32_t)(pointer->root_y - geom->y)};

      xcb_configure_window(Info::connection, DragInfo::window,
                           XCB_CONFIG_WINDOW_WIDTH | XCB_CONFIG_WINDOW_HEIGHT,
                           point);
    }
  } break;

  case XCB_BUTTON_RELEASE:
    log_verbose("XCB_BUTTON_RELEASE");

    xcb_ungrab_pointer(Info::connection, XCB_CURRENT_TIME);
    break;
  }

  xcb_flush(Info::connection);
}

void setup_bindings(void) {
  for (int i = 1; i < 4; i++) {
    xcb_grab_button(Info::connection, 0, Info::root,
                    XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE,
                    XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC, Info::root,
                    XCB_NONE, i, XCB_MOD_MASK_1);
  }

  xcb_grab_key(Info::connection, 1, Info::root, XCB_MOD_MASK_2, XCB_NO_SYMBOL,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

  xcb_flush(Info::connection);
}

int main(void) {
  int32_t scrno;
  Info::connection = xcb_connect(NULL, &scrno);

  if (xcb_connection_has_error(Info::connection))
    die_no_conn("xcb_connect error");

  Info::screen = xcb_setup_roots_iterator(xcb_get_setup(Info::connection)).data;

  if (!Info::screen)
    die("can't get screen");

  log("screen size: %dx%d", Info::screen->width_in_pixels,
      Info::screen->height_in_pixels);
  log("root window: %d", Info::screen->root);

  setup_bindings();

  while (true)
    event();
}
