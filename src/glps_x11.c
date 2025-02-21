/*
 Copyright 2025 Google LLC

 Licensed under the Apache License, Version 2.0 (the "License");
 you may not use this file except in compliance with the License.
 You may obtain a copy of the License at

      https://www.apache.org/licenses/LICENSE-2.0

 Unless required by applicable law or agreed to in writing, software
 distributed under the License is distributed on an "AS IS" BASIS,
 WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 See the License for the specific language governing permissions and
 limitations under the License.
 */

#include "internal/glps_x11.h"

void glps_x11_init(glps_WindowManager *wm)
{
    if (wm == NULL)
    {
        LOG_CRITICAL("Window Manager is NULL. Exiting..");
        exit(EXIT_FAILURE);
    }

    wm->display = XOpenDisplay(NULL);
    if (!wm->display)
    {
        LOG_CRITICAL("Failed to open X display\n");
        exit(EXIT_FAILURE);
    }

    wm->font = XLoadQueryFont(wm->display, "fixed");
    if (!wm->font)
    {
        LOG_CRITICAL("Failed to load system font\n");
        XCloseDisplay(wm->display);
        exit(EXIT_FAILURE);
    }
}

ssize_t glps_x11_window_create(glps_WindowManager *wm, const char *title,
                               int width, int height)
{

    if (wm == NULL || wm->display == NULL)
    {
        LOG_CRITICAL("Failed to create X11 window. Window manager and/or Display NULL.");
        exit(EXIT_FAILURE);
    }

    int screen = DefaultScreen(wm->display);

    wm->window = XCreateSimpleWindow(
        wm->display,
        RootWindow(wm->display, screen),
        10, 10, width, height, 1,
        BlackPixel(wm->display, screen),
        WhitePixel(wm->display, screen));
    XSetWindowBackground(wm->display, wm->window, 0xFFFFFF);
    XStoreName(wm->display, wm->window, title);

    wm->gc = XCreateGC(wm->display, wm->window, 0, NULL);

    wm->wm_delete_window = XInternAtom(wm->display, "WM_DELETE_WINDOW", False);
    XSetWMProtocols(wm->display, wm-window, &wm->wm_delete_window, 1);

    XSelectInput(wm->display, wm->window,
                 ExposureMask | ButtonPressMask | KeyPressMask | StructureNotifyMask);

    XMapWindow(wm->display, wm->window);

    return wm->window_count++;
}

void glps_x11_destroy(glps_WindowManager *wm);
void glps_x11_get_window_dimensions(glps_WindowManager *wm, size_t window_id,
                                    int *width, int *height);

void glps_x11_attach_to_clipboard(glps_WindowManager *wm, char *mime,
                                  char *data);

void glps_x11_get_from_clipboard(glps_WindowManager *wm, char *data,
                                 size_t data_size);
