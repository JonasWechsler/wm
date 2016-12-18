#ifndef __LOG_H__
#define __LOG_H__

#include <stdio.h>
#include "info.h"

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

#endif
