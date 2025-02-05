#ifndef GLPS_WIN32_H
#define GLPS_WIN32_H

void win32_init_window_class(glps_WindowManager *wm, const char *class_name);

void win32_window_create(glps_WindowManager *wm, HINSTANCE hInstance,
                         const char *class_name, int nCmdShow,
                         const char *title, int width, int height);
#endif
