#ifndef GLPS_WIN32_H
#define GLPS_WIN32_H

#include "glps_common.h"

void glps_win32_init(glps_WindowManager *wm);

ssize_t glps_win32_window_create(glps_WindowManager *wm, const char *title, int width,
                         int height);

void glps_win32_destroy(glps_WindowManager *wm);

HDC glps_win32_get_window_hdc(glps_WindowManager* wm, size_t window_id);

#endif
