#ifndef GLPS_WGL_CONTEXT_H
#define GLPS_WGL_CONTEXT_H
#include <glps_common.h>

void glps_wgl_init(glps_WindowManager *wm);
void glps_wgl_make_ctx_current(glps_WindowManager *wm, size_t window_id);
void *glps_wgl_get_proc_addr();
void glps_wgl_swap_buffers(glps_WindowManager *wm, size_t window_id);
void glps_wgl_destroy(glps_WindowManager *wm);

#endif
