#include <algorithm>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <xcb/xcb.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

struct Info {
  static xcb_connection_t *connection;
  static xcb_drawable_t root;
  static xcb_screen_t *screen;
};

xcb_connection_t *Info::connection;
xcb_drawable_t Info::root;
xcb_screen_t *Info::screen;

enum DragAction { MOVE, RESIZE, NONE };

struct DragInfo {
  static xcb_drawable_t window;
  static DragAction action;
};

xcb_drawable_t DragInfo::window;
DragAction DragInfo::action;

#define log(msg, args...)                                                      \
  do {                                                                         \
    printf("[%s - %s:%d]" msg "\n", __func__, __FILE__, __LINE__, ##args);     \
  } while (0)

#define VERBOSE

#ifdef VERBOSE
#define log_verbose(msg, args...) log(msg, ##args)
#else
#define log_verbose(msgm args...)
#endif

#define die_no_conn(msg, args...)                                              \
  do {                                                                         \
    log("(EXIT)" msg, ##args);                                                 \
    exit(1);                                                                   \
  } while (0)

#define die(msg, args...)                                                      \
  do {                                                                         \
    xcb_disconnect(Info::connection);                                          \
    die_no_conn(msg, ##args);                                                  \
  } while (0)

xcb_get_geometry_reply_t *geom;

void event(void) {
  xcb_generic_event_t *event = xcb_wait_for_event(Info::connection);

  log_verbose("event response %d %s", event->response_type & ~0x80,
              (DragInfo::action == MOVE)
                  ? "(move)"
                  : (DragInfo::action == RESIZE) ? "(resize)" : "");

  switch (event->response_type & ~0x80) {

  case XCB_BUTTON_PRESS: {
    log_verbose("XCB_BUTTON_PRESS");

    xcb_button_press_event_t *e = (xcb_button_press_event_t *)event;
    DragInfo::window = e->child;

    const uint32_t mode[1] = {XCB_STACK_MODE_ABOVE};
    xcb_configure_window(Info::connection, DragInfo::window,
                         XCB_CONFIG_WINDOW_STACK_MODE, mode);

    geom = xcb_get_geometry_reply(
        Info::connection, xcb_get_geometry(Info::connection, DragInfo::window),
        NULL);

    if (e->detail == 1) {
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

    xcb_query_pointer_reply_t *pointer;
    pointer = xcb_query_pointer_reply(
        Info::connection, xcb_query_pointer(Info::connection, Info::root), 0);

    auto geom = xcb_get_geometry_reply(
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

int main(void) {
  int32_t scrno;
  Info::connection = xcb_connect(NULL, &scrno);

  if (xcb_connection_has_error(Info::connection))
    die_no_conn("xcb_connect error");

  Info::screen = xcb_setup_roots_iterator(xcb_get_setup(Info::connection)).data;

  if (!Info::screen)
    die("can't get Info::screen");

  Info::root = Info::screen->root;

  log("screen size: %dx%d", Info::screen->width_in_pixels,
      Info::screen->height_in_pixels);
  log("root window: %d", Info::screen->root);

  auto mask = XCB_EVENT_MASK_BUTTON_PRESS | XCB_EVENT_MASK_BUTTON_RELEASE;
  auto asyn = XCB_GRAB_MODE_ASYNC;

  // const uint32_t cwa = XCB_EVENT_MASK_SUBSTRUCTURE_REDIRECT
  //    | XCB_EVENT_MASK_SUBSTRUCTURE_NOTIFY
  //    | XCB_EVENT_MASK_STRUCTURE_NOTIFY;

  // xcb_change_window_attributes(Info::connection, Info::root,
  // XCB_CW_EVENT_MASK,
  //        &cwa);
  //
  // Add mouse buttons 1, 2, 3
  xcb_grab_button(Info::connection, 0, Info::root, mask, asyn, asyn, Info::root,
                  XCB_NONE, 1, XCB_MOD_MASK_1);
  xcb_grab_button(Info::connection, 0, Info::root, mask, asyn, asyn, Info::root,
                  XCB_NONE, 2, XCB_MOD_MASK_1);
  xcb_grab_button(Info::connection, 0, Info::root, mask, asyn, asyn, Info::root,
                  XCB_NONE, 3, XCB_MOD_MASK_1);

  xcb_grab_key(Info::connection, 1, Info::root, XCB_MOD_MASK_2, XCB_NO_SYMBOL,
               XCB_GRAB_MODE_ASYNC, XCB_GRAB_MODE_ASYNC);

  xcb_flush(Info::connection);

  while (true)
    event();
}
