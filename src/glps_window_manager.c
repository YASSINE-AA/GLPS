#include "glps_window_manager.h"
#include "glps_wayland.h"
#include <EGL/eglplatform.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <xkbcommon/xkbcommon.h>

void glps_wm_set_mouse_enter_callback(
    glps_WindowManager *wm,
    void (*mouse_enter_callback)(size_t window_id, double mouse_x,
                                 double mouse_y, void *data),
    void *data) {

  if (wm == NULL || mouse_enter_callback == NULL) {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_enter_callback = mouse_enter_callback;
  wm->callbacks.mouse_move_data = data;
}

void glps_wm_set_mouse_leave_callback(
    glps_WindowManager *wm,
    void (*mouse_leave_callback)(size_t window_id, void *data), void *data) {

  if (wm == NULL || mouse_leave_callback == NULL) {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_leave_callback = mouse_leave_callback;
  wm->callbacks.mouse_leave_data = data;
}

void glps_wm_set_mouse_move_callback(
    glps_WindowManager *wm,
    void (*mouse_move_callback)(size_t window_id, double mouse_x,
                                double mouse_y, void *data),
    void *data) {

  if (wm == NULL || mouse_move_callback == NULL) {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_move_callback = mouse_move_callback;
  wm->callbacks.mouse_move_data = data;
}

void glps_wm_set_mouse_click_callback(
    glps_WindowManager *wm,
    void (*mouse_click_callback)(size_t window_id, bool state, void *data),
    void *data) {

  if (wm == NULL || mouse_click_callback == NULL) {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_click_callback = mouse_click_callback;
  wm->callbacks.mouse_click_data = data;
}

void glps_wm_set_scroll_callback(
    glps_WindowManager *wm,
    void (*mouse_scroll_callback)(size_t window_id, GLPS_SCROLL_AXES axe,
                                  GLPS_SCROLL_SOURCE source, double value,
                                  int discrete, bool is_stopped, void *data),
    void *data) {

  if (wm == NULL || mouse_scroll_callback == NULL) {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.mouse_scroll_callback = mouse_scroll_callback;
  wm->callbacks.mouse_scroll_data = data;
}

void glps_wm_set_keyboard_enter_callback(
    glps_WindowManager *wm,
    void (*keyboard_enter_callback)(size_t window_id, void *data), void *data) {

  if (wm == NULL || keyboard_enter_callback == NULL) {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.keyboard_enter_callback = keyboard_enter_callback;
  wm->callbacks.keyboard_enter_data = data;
}

void glps_wm_set_keyboard_callback(glps_WindowManager *wm,
                                   void (*keyboard_callback)(size_t window_id,
                                                             bool state,
                                                             const char *value,
                                                             void *data),
                                   void *data) {

  if (wm == NULL || keyboard_callback == NULL) {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.keyboard_callback = keyboard_callback;
  wm->callbacks.keyboard_data = data;
}

void glps_wm_set_keyboard_leave_callback(
    glps_WindowManager *wm,
    void (*keyboard_leave_callback)(size_t window_id, void *data), void *data) {

  if (wm == NULL || keyboard_leave_callback == NULL) {
    LOG_CRITICAL("Window Manager and/or Callback function NULL.");
    return;
  }

  wm->callbacks.keyboard_leave_callback = keyboard_leave_callback;
  wm->callbacks.keyboard_leave_data = data;
}

void glps_wm_set_touch_callback(
    glps_WindowManager *wm,
    void (*touch_callback)(size_t window_id, int id, double touch_x,
                           double touch_y, bool state, double major,
                           double minor, double orientation, void *data),
    void *data) {

  if (wm == NULL || touch_callback == NULL) {
    LOG_ERROR("Window Manager and/or Touch Callback NULL");
    return;
  }

  wm->callbacks.touch_callback = touch_callback;
  wm->callbacks.touch_data = data;
}

void glps_wm_attach_to_clipboard(glps_WindowManager *wm, char *mime,
                                 char *data) {

  glps_WaylandContext *context = NULL;
  if (wm == NULL || (context = __get_wl_context(wm)) == NULL) {
    LOG_ERROR("Couldn't attach data to clipboard, context is NULL.");
    return;
  }

  memset(&wm->clipboard, 0, sizeof(wm->clipboard));
  strcat(wm->clipboard.buff, data);
  strcat(wm->clipboard.mime_type, mime);
  context->data_src =
      wl_data_device_manager_create_data_source(context->data_dvc_manager);
  wl_data_source_add_listener(context->data_src, &data_source_listener, wm);
  wl_data_source_offer(context->data_src, "text/plain");
  wl_data_source_offer(context->data_src, "text/html");
  wl_data_device_set_selection(context->data_dvc, context->data_src,
                               context->keyboard_serial);
}

void glps_wm_get_from_clipboard(glps_WindowManager *wm, char *data,
                                size_t data_size) {

  if (wm == NULL || data == NULL) {
    LOG_ERROR("Window Manager and/or data NULL.");
    return;
  }

  memset(data, 0, data_size);
  strncpy(data, wm->clipboard.buff, data_size - 1);
  data[data_size - 1] = '\0';
}

void glps_wm_start_drag_n_drop(
    glps_WindowManager *wm, size_t origin_window_id,
    void (*drag_n_drop_callback)(size_t origin_window_id, char *mime,
                                 char *buff, void *data),
    void *data) {
  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  glps_WaylandContext *ctx = (glps_WaylandContext *)__get_wl_context(wm);
  if (ctx == NULL) {
    LOG_ERROR("Wayland context is NULL.");
    return;
  }

  wm->callbacks.drag_n_drop_callback = drag_n_drop_callback;
  wm->callbacks.drag_n_drop_data = data;
  ctx->current_drag_n_drop_window = origin_window_id;

  struct wl_data_source *source =
      wl_data_device_manager_create_data_source(ctx->data_dvc_manager);
  wl_data_source_add_listener(source, &data_source_listener, wm);
  wl_data_source_offer(source, "text/plain");

  wl_data_source_set_actions(source,
                             WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE |
                                 WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);

  struct wl_surface *icon = NULL;
  wl_data_device_start_drag(ctx->data_dvc, source,
                            wm->windows[origin_window_id]->wl_surface, icon,
                            wm->pointer_event.serial);
}

void glps_wm_swap_interval(glps_WindowManager *wm, unsigned int swap_interval) {
  eglSwapInterval(wm->egl_ctx->dpy, 0);
}

void glps_wm_swap_buffers(glps_WindowManager *wm, size_t window_id) {
  eglSwapBuffers(wm->egl_ctx->dpy, wm->windows[window_id]->egl_surface);
}

void glps_wm_window_set_resize_callback(
    glps_WindowManager *wm,
    void (*window_resize_callback)(size_t window_id, int width, int height,
                                   void *data),
    void *data) {

  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  wm->callbacks.window_resize_callback = window_resize_callback;
  wm->callbacks.window_resize_data = data;
}

void glps_wm_window_set_frame_update_callback(
    glps_WindowManager *wm,
    void (*window_frame_update_callback)(size_t window_id, void *data),
    void *data) {

  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  wm->callbacks.window_frame_update_callback = window_frame_update_callback;
  wm->callbacks.window_frame_update_data = data;
}

void glps_wm_window_set_close_callback(
    glps_WindowManager *wm,
    void (*window_close_callback)(size_t window_id, void *data), void *data) {

  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  wm->callbacks.window_close_callback = window_close_callback;
  wm->callbacks.window_close_data = data;
}

static void _create_ctx(glps_WindowManager *wm) {
  static const EGLint context_attribs[] = {
      EGL_CONTEXT_MAJOR_VERSION,
      4,
      EGL_CONTEXT_MINOR_VERSION,
      5,
      EGL_CONTEXT_OPENGL_PROFILE_MASK_KHR,
      EGL_CONTEXT_OPENGL_CORE_PROFILE_BIT_KHR,
      EGL_NONE};

  wm->egl_ctx->ctx = eglCreateContext(wm->egl_ctx->dpy, wm->egl_ctx->conf,
                                      EGL_NO_CONTEXT, context_attribs);
  if (wm->egl_ctx->ctx == EGL_NO_CONTEXT) {
    fprintf(stderr, "Failed to create EGL context\n");
    exit(EXIT_FAILURE);
  }
}

static void _init_egl(glps_WindowManager *wm) {
  wm->egl_ctx = malloc(sizeof(glps_EGLContext));

  EGLint config_attribs[] = {EGL_SURFACE_TYPE,
                             EGL_WINDOW_BIT,
                             EGL_RED_SIZE,
                             8,
                             EGL_GREEN_SIZE,
                             8,
                             EGL_BLUE_SIZE,
                             8,
                             EGL_ALPHA_SIZE,
                             8,
                             EGL_RENDERABLE_TYPE,
                             EGL_OPENGL_BIT,
                             EGL_NONE};

  EGLint major, minor, n;

  wm->egl_ctx->dpy =
      eglGetDisplay((EGLNativeDisplayType)wm->wayland_ctx->wl_display);
  assert(wm->egl_ctx->dpy);

  if (!eglInitialize(wm->egl_ctx->dpy, &major, &minor)) {
    LOG_ERROR("Failed to initialize EGL");
    exit(EXIT_FAILURE);
  }

  LOG_INFO("EGL initialized successfully (version %d.%d)", major, minor);

  if (!eglChooseConfig(wm->egl_ctx->dpy, config_attribs, &wm->egl_ctx->conf, 1,
                       &n) ||
      n != 1) {
    LOG_ERROR("Failed to choose a valid EGL config");
    exit(EXIT_FAILURE);
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    LOG_ERROR("Failed to bind OpenGL API");
    exit(EXIT_FAILURE);
  }
  EGLint error = eglGetError();
  if (error != EGL_SUCCESS) {
    LOG_ERROR("EGL error: %x", error);
  }
}
glps_WindowManager *glps_wm_init(void) {

  glps_WindowManager *wm = malloc(sizeof(glps_WindowManager));
  *wm = (glps_WindowManager){0};
  if (!wm) {
    LOG_ERROR("Failed to allocate memory for glps_WindowManager");
    return NULL;
  }
#ifdef GLPS_USE_WAYLAND

  wm->windows = malloc(sizeof(glps_WaylandWindow *) * MAX_WINDOWS);
  if (!wm->windows) {
    LOG_ERROR("Failed to allocate memory for windows array");
    free(wm);
    return NULL;
  }

  wm->wayland_ctx = malloc(sizeof(glps_WaylandContext));
  *wm->wayland_ctx = (glps_WaylandContext){0};
  if (!wm->wayland_ctx) {
    LOG_ERROR("Failed to allocate memory for Wayland context");
    free(wm->windows);
    free(wm);
    return NULL;
  }

  wm->window_count = 0;
  wm->wayland_ctx->wl_touch = NULL;
  wm->wayland_ctx->wl_pointer = NULL;
  wm->wayland_ctx->wl_keyboard = NULL;
  wm->wayland_ctx->xkb_state = NULL;
  wm->wayland_ctx->xkb_keymap = NULL;
  wm->wayland_ctx->xkb_context = NULL;
  wm->wayland_ctx->decoration_manager = NULL;
  wm->wayland_ctx->xkb_context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);

  wm->wayland_ctx->wl_display = wl_display_connect(NULL);
  if (!wm->wayland_ctx->wl_display) {
    LOG_ERROR("Failed to connect to Wayland display");
    free(wm->wayland_ctx);
    free(wm->windows);
    free(wm);
    return NULL;
  }

  wm->wayland_ctx->wl_registry =
      wl_display_get_registry(wm->wayland_ctx->wl_display);
  if (!wm->wayland_ctx->wl_registry) {
    LOG_ERROR("Failed to get Wayland registry");
    wl_display_disconnect(wm->wayland_ctx->wl_display);
    free(wm->wayland_ctx);
    free(wm->windows);
    free(wm);
    return NULL;
  }

  wl_registry_add_listener(wm->wayland_ctx->wl_registry, &registry_listener,
                           wm);

  wl_display_roundtrip(wm->wayland_ctx->wl_display);

  if (wm->wayland_ctx->xdg_wm_base) {
    xdg_wm_base_add_listener(wm->wayland_ctx->xdg_wm_base,
                             &xdg_wm_base_listener, NULL);
  } else {
    LOG_ERROR("xdg_wm_base protocol not supported by compositor");
  }

  if (!wm->wayland_ctx->decoration_manager) {
    LOG_ERROR("xdg-decoration protocol not supported by compositor");
  }

  if (!wm->wayland_ctx->wl_compositor || !wm->wayland_ctx->xdg_wm_base) {
    LOG_ERROR("Failed to retrieve Wayland compositor or xdg_wm_base");
    wl_registry_destroy(wm->wayland_ctx->wl_registry);
    wl_display_disconnect(wm->wayland_ctx->wl_display);
    free(wm->wayland_ctx);
    free(wm->windows);
    free(wm);
    return NULL;
  }

  _init_egl(wm);

#elif defined(GLPS_USE_WIN32)
  LOG_CRITICAL("USING WIN32");
  exit(1);
#endif

  return wm;
}

void glps_wm_set_window_ctx_curr(glps_WindowManager *wm, size_t window_id) {
  if (!eglMakeCurrent(wm->egl_ctx->dpy, wm->windows[window_id]->egl_surface,
                      wm->windows[window_id]->egl_surface, wm->egl_ctx->ctx)) {
    EGLint error = eglGetError();
    LOG_ERROR("eglMakeCurrent failed: 0x%x", error);
    if (error == EGL_BAD_DISPLAY)
      LOG_ERROR("Invalid EGL display");
    if (error == EGL_BAD_SURFACE)
      LOG_ERROR("Invalid draw or read surface");
    if (error == EGL_BAD_CONTEXT)
      LOG_ERROR("Invalid EGL context");
    if (error == EGL_BAD_MATCH)
      LOG_ERROR("Context or surface attributes mismatch");
    exit(EXIT_FAILURE);
  }
}

void glps_wm_window_get_dimensions(glps_WindowManager *wm, size_t window_id,
                                   int *width, int *height) {

  if (wm == NULL) {
    LOG_ERROR("Couldn't get window dimensions. Window Manager NULL. ");
    return;
  }

  glps_WaylandWindow *window = (glps_WaylandWindow *)wm->windows[window_id];

  *width = window->properties.width;
  *height = window->properties.height;
}

void *glps_get_proc_addr() { return eglGetProcAddress; }

size_t glps_wm_window_create(glps_WindowManager *wm, const char *title,
                             int width, int height) {
  glps_WaylandWindow *window = malloc(sizeof(glps_WaylandWindow));

  window->wl_surface =
      wl_compositor_create_surface(wm->wayland_ctx->wl_compositor);
  if (!window->wl_surface) {
    LOG_ERROR("Failed to create wayland surface");
    exit(EXIT_FAILURE);
  }

  window->properties.width = width;
  window->properties.height = height;

  window->fps_start_time = (struct timespec){0};
  window->fps_is_init = false;

  window->xdg_surface = xdg_wm_base_get_xdg_surface(
      wm->wayland_ctx->xdg_wm_base, window->wl_surface);

  if (!window->xdg_surface) {
    LOG_ERROR("Failed to create XDG surface");
    exit(EXIT_FAILURE);
  }

  if (xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener,
                               wm) == -1) {
    LOG_ERROR("Failed to add XDG surface listener");
    exit(EXIT_FAILURE);
  }

  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  if (!window->xdg_toplevel) {
    LOG_ERROR("Failed to create toplevel");
    exit(EXIT_FAILURE);
  }

  xdg_toplevel_set_title(window->xdg_toplevel, title);
  strcpy(window->properties.title, title);
  xdg_toplevel_add_listener(window->xdg_toplevel, &toplevel_listener, wm);
  if (wm->wayland_ctx->decoration_manager != NULL) {

    window->zxdg_toplevel_decoration =
        zxdg_decoration_manager_v1_get_toplevel_decoration(
            wm->wayland_ctx->decoration_manager, window->xdg_toplevel);
    zxdg_toplevel_decoration_v1_set_mode(
        window->zxdg_toplevel_decoration,
        ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
  }

  wl_surface_commit(window->wl_surface);

  wl_display_roundtrip(wm->wayland_ctx->wl_display);

  window->egl_window = wl_egl_window_create(
      window->wl_surface, window->properties.width, window->properties.height);
  if (!window->egl_window) {
    LOG_ERROR("Failed to create EGL window");
    exit(EXIT_FAILURE);
  }

  window->egl_surface =
      eglCreateWindowSurface(wm->egl_ctx->dpy, wm->egl_ctx->conf,
                             (NativeWindowType)window->egl_window, NULL);
  if (window->egl_surface == EGL_NO_SURFACE) {
    LOG_ERROR("Failed to create EGL surface");
    exit(EXIT_FAILURE);
  }

  wm->windows[wm->window_count] = window;

  if (wm->window_count == 0) {
    _create_ctx(wm);
    glps_wm_set_window_ctx_curr(wm, 0);
  }

  // setup frame callback
  frame_callback_args *frame_args =
      (frame_callback_args *)malloc(sizeof(frame_callback_args));
  window->frame_callback = wl_surface_frame(window->wl_surface);
  frame_args->wm = wm;
  frame_args->window_id = wm->window_count;
  window->frame_args = (void *)frame_args;

  wl_callback_add_listener(window->frame_callback, &frame_callback_listener,
                           frame_args);

  return wm->window_count++;
}

static void _cleanup_egl(glps_WindowManager *wm) {

  if (wm->egl_ctx->ctx) {
    eglDestroyContext(wm->egl_ctx->dpy, wm->egl_ctx->ctx);
    wm->egl_ctx->ctx = EGL_NO_CONTEXT;
  }
  if (wm->egl_ctx->dpy) {
    eglTerminate(wm->egl_ctx->dpy);
    wm->egl_ctx->dpy = EGL_NO_DISPLAY;
  }
  free(wm->egl_ctx);
  wm->egl_ctx = NULL;
}

void glps_wm_window_destroy(glps_WindowManager *wm, size_t window_id) {
  if (wm == NULL || window_id >= wm->window_count ||
      wm->windows[window_id] == NULL) {
    LOG_ERROR("Invalid window ID or window manager is NULL.");
    return;
  }

  glps_WaylandWindow *window = wm->windows[window_id];
  if (window->frame_args != NULL) {
    free(window->frame_args);
    window->frame_args = NULL;
  }

  if (window->zxdg_toplevel_decoration != NULL) {
    zxdg_toplevel_decoration_v1_destroy(window->zxdg_toplevel_decoration);
    window->zxdg_toplevel_decoration = NULL;
  }

  if (window->frame_callback != NULL) {
    wl_callback_destroy(window->frame_callback);
    window->frame_callback = NULL;
  }

  eglDestroySurface(wm->egl_ctx->dpy, window->egl_surface);
  wl_egl_window_destroy(window->egl_window);

  xdg_toplevel_destroy(window->xdg_toplevel);
  xdg_surface_destroy(window->xdg_surface);
  wl_surface_destroy(window->wl_surface);

  free(window);

  wm->windows[window_id] = NULL;

  for (size_t i = window_id; i < wm->window_count - 1; ++i) {
    wm->windows[i] = wm->windows[i + 1];
  }
  if (wm->window_count > 0)
    wm->window_count--;

  if (wm->window_count == 0) {
    LOG_INFO("All windows destroyed. Exiting program.");
    glps_wm_destroy(wm);
   // exit(EXIT_SUCCESS);
  }
}
double glps_wm_get_fps(glps_WindowManager *wm, size_t window_id) {

  if (!wm->windows[window_id]->fps_is_init) {
    clock_gettime(CLOCK_MONOTONIC, &wm->windows[window_id]->fps_start_time);
    wm->windows[window_id]->fps_is_init = true;
    return 0;
  } else {
    struct timespec end_time = {0};
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double seconds =
        end_time.tv_sec - wm->windows[window_id]->fps_start_time.tv_sec;
    double nanoseconds =
        end_time.tv_nsec - wm->windows[window_id]->fps_start_time.tv_nsec;

    if (nanoseconds < 0) {
      seconds--;
      nanoseconds += 1000000000L;
    }
    wm->windows[window_id]->fps_start_time = end_time;
    return (double)1.0 / ((seconds + nanoseconds) / 1e9);
  }
}

bool glps_wm_should_close(glps_WindowManager *wm) {
  if (wl_display_dispatch(wm->wayland_ctx->wl_display) == -1)
    return true;
  else if (wm->window_count == 0)
    return true;

  return false;
}

void glps_wm_destroy(glps_WindowManager *wm) {

  if (wm == NULL) {
    return;
  }

  for (size_t i = 0; i < wm->window_count; ++i) {
    if (wm->windows[i] != NULL) {
      glps_wm_window_destroy(wm, i);
    }
  }
  _cleanup_egl(wm);
  _cleanup_wl(wm);
  if (wm != NULL) {
    free(wm);
    wm = NULL;
  }
}
