#ifndef EGL_CONTEXT_H
#define EGL_CONTEXT_H

#include <glps_common.h>

void egl_init(glps_WindowManager *wm);
void egl_create_ctx(glps_WindowManager *wm);
void egl_make_ctx_current(glps_WindowManager *wm, size_t window_id);
void *egl_get_proc_addr();
void egl_swap_buffers(glps_WindowManager *wm, size_t window_id);
void egl_destroy(glps_WindowManager *wm);

#endif
