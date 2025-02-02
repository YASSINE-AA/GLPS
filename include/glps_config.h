#ifndef GLPS_CONFIG_H
#define GLPS_CONFIG_H

#ifdef __linux__
#include "utils/logger/pico_logger.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GLPS_USE_LINUX

static inline const char *glps_detect_platform() {
  const char *session_type = NULL;
  session_type = getenv("XDG_SESSION_TYPE");
  return session_type;
}

#define GLPS_INIT() glps_detect_platform()
#endif

#ifdef _WIN32
#define GLPS_USE_WIN32
#endif

#endif
