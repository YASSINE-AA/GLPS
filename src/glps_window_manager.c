#include "../include/glps_window_manager.h"
#include "../include/glps_opengl.h"
#include <EGL/eglplatform.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <wayland-client-core.h>
#include <wayland-client-protocol.h>
#include <wayland-egl-core.h>
#include <xkbcommon/xkbcommon.h>

static glps_WaylandContext *__get_wl_context(glps_WindowManager *wm) {
  if (wm == NULL || wm->wayland_ctx == NULL) {
    LOG_WARNING("Window manager and/or Wayland context is NULL.");
    return NULL;
  }
  return wm->wayland_ctx;
}

static ssize_t __get_window_id_from_surface(glps_WindowManager *wm,
                                            struct wl_surface *surface) {

  if (wm == NULL || surface == NULL) {
    LOG_ERROR("Couldn't get window id from surface, Window manager and/or "
              "Surface is NULL.");
    return -1;
  }

  for (size_t i = 0; i < wm->window_count; ++i) {
    if (surface == wm->windows[i]->wl_surface)
      return i;
  }

  return -1;
}

static ssize_t __get_window_id_from_xdg_surface(glps_WindowManager *wm,
                                                struct xdg_surface *surface) {

  if (wm == NULL || surface == NULL) {
    LOG_ERROR("Couldn't get window id from surface, Window manager and/or "
              "Surface is NULL.");
    return -1;
  }

  for (size_t i = 0; i < wm->window_count; ++i) {
    if (surface == wm->windows[i]->xdg_surface)
      return i;
  }

  return -1;
}

static ssize_t
__get_window_id_from_xdg_toplevel(glps_WindowManager *wm,
                                  struct xdg_toplevel *toplevel) {

  if (wm == NULL || toplevel == NULL) {
    LOG_ERROR("Couldn't get window id from toplevel, Window manager and/or "
              "toplevel is NULL.");
    return -1;
  }

  for (size_t i = 0; i < wm->window_count; ++i) {
    if (toplevel == wm->windows[i]->xdg_toplevel)
      return i;
  }

  return -1;
}

void glps_wm_window_resize(glps_WindowManager *wm, size_t window_id, int width,
                           int height, int dx, int dy) {
  if (wm == NULL) {
    LOG_ERROR("Couldn't resize window. Window Manager is NULL.");
    return;
  }

  glps_WaylandContext *ctx = __get_wl_context(wm);

  if (ctx == NULL) {
    LOG_ERROR("Couldn't resize window. Wayland Context is NULL.");
    return;
  }

  // wl_egl_window_resize(wm->windows[window_id]->egl_surface, width, height,
  // dx,
  //                     dy);
  xdg_toplevel_resize(wm->windows[window_id]->xdg_toplevel, ctx->wl_seat,
                      wm->windows[window_id]->serial, 10);
}

static void xdg_wm_base_ping(void *data, struct xdg_wm_base *xdg_wm_base,
                             uint32_t serial) {
  xdg_wm_base_pong(xdg_wm_base, serial);
}

static const struct xdg_wm_base_listener xdg_wm_base_listener = {
    .ping = xdg_wm_base_ping,
};

static const struct wl_callback_listener frame_callback_listener;
static void wl_pointer_enter(void *data, struct wl_pointer *wl_pointer,
                             uint32_t serial, struct wl_surface *surface,
                             wl_fixed_t surface_x, wl_fixed_t surface_y) {
  glps_WindowManager *context = (glps_WindowManager *)data;

  ssize_t window_id = __get_window_id_from_surface(context, surface);
  if (window_id < 0) {
    LOG_ERROR("Origin window id is invalid.");
    return;
  }

  LOG_INFO("window id %ld", window_id);
  context->pointer_event.event_mask |= POINTER_EVENT_ENTER;
  context->pointer_event.serial = serial;
  context->pointer_event.surface_x = surface_x,
  context->pointer_event.surface_y = surface_y;

  glps_WaylandContext *wayland_context = __get_wl_context(context);

  if (wayland_context == NULL) {
    LOG_ERROR("Couldn't fetch wayland context.");
    return;
  }

  wayland_context->mouse_window_id = (size_t)window_id;
}

static void wl_pointer_leave(void *data, struct wl_pointer *wl_pointer,
                             uint32_t serial, struct wl_surface *surface) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.serial = serial;
  context->pointer_event.event_mask |= POINTER_EVENT_LEAVE;
}

static void wl_pointer_motion(void *data, struct wl_pointer *wl_pointer,
                              uint32_t time, wl_fixed_t surface_x,
                              wl_fixed_t surface_y) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_MOTION;
  context->pointer_event.time = time;
  context->pointer_event.surface_x = surface_x,
  context->pointer_event.surface_y = surface_y;
}

static void wl_pointer_button(void *data, struct wl_pointer *wl_pointer,
                              uint32_t serial, uint32_t time, uint32_t button,
                              uint32_t state) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_BUTTON;
  context->pointer_event.time = time;
  context->pointer_event.serial = serial;
  context->pointer_event.button = button, context->pointer_event.state = state;
}

static void wl_pointer_axis(void *data, struct wl_pointer *wl_pointer,
                            uint32_t time, uint32_t axis, wl_fixed_t value) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_AXIS;
  context->pointer_event.time = time;
  context->pointer_event.axes[axis].valid = true;
  context->pointer_event.axes[axis].value = value;
}

static void wl_pointer_axis_source(void *data, struct wl_pointer *wl_pointer,
                                   uint32_t axis_source) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_AXIS_SOURCE;
  context->pointer_event.axis_source = axis_source;
}

static void wl_pointer_axis_stop(void *data, struct wl_pointer *wl_pointer,
                                 uint32_t time, uint32_t axis) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.time = time;
  context->pointer_event.event_mask |= POINTER_EVENT_AXIS_STOP;
  context->pointer_event.axes[axis].valid = true;
}

static void wl_pointer_axis_discrete(void *data, struct wl_pointer *wl_pointer,
                                     uint32_t axis, int32_t discrete) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  context->pointer_event.event_mask |= POINTER_EVENT_AXIS_DISCRETE;
  context->pointer_event.axes[axis].valid = true;
  context->pointer_event.axes[axis].discrete = discrete;
}

static void wl_pointer_frame(void *data, struct wl_pointer *wl_pointer) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  struct pointer_event *event = &context->pointer_event;
  glps_WaylandContext *wayland_context = __get_wl_context(context);

  if (wayland_context == NULL) {
    LOG_ERROR("Couldn't fetch wayland context.");
    return;
  }
  if (event->event_mask & POINTER_EVENT_ENTER) {
    // Mouse enter callback
    if (context->callbacks.mouse_enter_callback) {
      context->callbacks.mouse_enter_callback(
          wayland_context->mouse_window_id,
          wl_fixed_to_double(event->surface_x),
          wl_fixed_to_double(event->surface_y),
          context->callbacks.mouse_enter_data);
    }
  }

  if (event->event_mask & POINTER_EVENT_LEAVE) {
    // Mouse leave callback
    if (context->callbacks.mouse_leave_callback) {
      context->callbacks.mouse_leave_callback(
          wayland_context->mouse_window_id,
          context->callbacks.mouse_leave_data);
    }
  }

  if (event->event_mask & POINTER_EVENT_MOTION) {
    // Mouse move callback
    if (context->callbacks.mouse_move_callback) {
      context->callbacks.mouse_move_callback(
          wayland_context->mouse_window_id,
          wl_fixed_to_double(event->surface_x),
          wl_fixed_to_double(event->surface_y),
          context->callbacks.mouse_move_data);
    }
  }

  if (event->event_mask & POINTER_EVENT_BUTTON) {
    char *state = event->state == WL_POINTER_BUTTON_STATE_RELEASED ? "released"
                                                                   : "pressed";

    // Mouse click callback
    if (context->callbacks.mouse_click_callback) {
      context->callbacks.mouse_click_callback(
          wayland_context->mouse_window_id,
          event->state == WL_POINTER_BUTTON_STATE_RELEASED ? false : true,
          context->callbacks.mouse_click_data);
    }
  }

  uint32_t axis_events = POINTER_EVENT_AXIS | POINTER_EVENT_AXIS_SOURCE |
                         POINTER_EVENT_AXIS_STOP | POINTER_EVENT_AXIS_DISCRETE;

  if (event->event_mask & axis_events) {
    for (size_t i = 0; i < 2; ++i) {
      if (!event->axes[i].valid) {
        continue;
      }
      // Mouse scroll callback.
      if (context->callbacks.mouse_scroll_callback) {
        GLPS_SCROLL_AXES axis_name[2] = {
            [WL_POINTER_AXIS_VERTICAL_SCROLL] = GLPS_SCROLL_V_AXIS,
            [WL_POINTER_AXIS_HORIZONTAL_SCROLL] = GLPS_SCROLL_H_AXIS,
        };

        GLPS_SCROLL_SOURCE axis_source[4] = {
            [WL_POINTER_AXIS_SOURCE_WHEEL] = GLPS_SCROLL_SOURCE_WHEEL,
            [WL_POINTER_AXIS_SOURCE_FINGER] = GLPS_SCROLL_SOURCE_FINGER,
            [WL_POINTER_AXIS_SOURCE_CONTINUOUS] = GLPS_SCROLL_SOURCE_CONTINUOUS,
            [WL_POINTER_AXIS_SOURCE_WHEEL_TILT] = GLPS_SCROLL_SOURCE_WHEEL_TILT,
        };

        GLPS_SCROLL_AXES axe = axis_name[i];
        GLPS_SCROLL_SOURCE source =
            event->event_mask & POINTER_EVENT_AXIS_SOURCE
                ? axis_source[event->axis_source]
                : GLPS_SCROLL_SOURCE_OTHER;
        double value = event->event_mask & POINTER_EVENT_AXIS
                           ? wl_fixed_to_double(event->axes[i].value)
                           : 0.0f;
        int discrete = event->event_mask & POINTER_EVENT_AXIS_DISCRETE
                           ? event->axes[i].discrete
                           : -1;
        bool is_stopped = event->event_mask & POINTER_EVENT_AXIS_STOP;

        context->callbacks.mouse_scroll_callback(
            wayland_context->mouse_window_id,

            axe, source, value, discrete, is_stopped,
            context->callbacks.mouse_scroll_data);
      }
    }
  }
  memset(event, 0, sizeof(*event));
}

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

static const struct wl_pointer_listener wl_pointer_listener = {
    .enter = wl_pointer_enter,
    .leave = wl_pointer_leave,
    .motion = wl_pointer_motion,
    .button = wl_pointer_button,
    .axis = wl_pointer_axis,
    .frame = wl_pointer_frame,
    .axis_source = wl_pointer_axis_source,
    .axis_stop = wl_pointer_axis_stop,
    .axis_discrete = wl_pointer_axis_discrete,
};

static void wl_keyboard_keymap(void *data, struct wl_keyboard *wl_keyboard,
                               uint32_t format, int32_t fd, uint32_t size) {
  glps_WaylandContext *context = __get_wl_context(data);
  if (context == NULL)
    return;
  assert(format == WL_KEYBOARD_KEYMAP_FORMAT_XKB_V1);

  char *map_shm = mmap(NULL, size, PROT_READ, MAP_SHARED, fd, 0);
  assert(map_shm != MAP_FAILED);
  struct xkb_keymap *xkb_keymap = xkb_keymap_new_from_string(
      context->xkb_context, map_shm, XKB_KEYMAP_FORMAT_TEXT_V1,
      XKB_KEYMAP_COMPILE_NO_FLAGS);

  munmap(map_shm, size);
  close(fd);

  struct xkb_state *xkb_state = xkb_state_new(xkb_keymap);
  xkb_keymap_unref(context->xkb_keymap);
  xkb_state_unref(context->xkb_state);
  context->xkb_keymap = xkb_keymap;
  context->xkb_state = xkb_state;
}

static void wl_keyboard_enter(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t serial, struct wl_surface *surface,
                              struct wl_array *keys) {
  glps_WaylandContext *context = __get_wl_context(data);
  if (context == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm->callbacks.keyboard_enter_callback != NULL) {
    wm->callbacks.keyboard_enter_callback(context->keyboard_window_id,
                                          wm->callbacks.keyboard_enter_data);
  }
  ssize_t window_id = __get_window_id_from_surface(wm, surface);

  if (window_id < 0) {

    LOG_ERROR("Origin window id is invalid.");
    return;
  }
  context->keyboard_serial = serial;
  context->keyboard_window_id = (size_t)window_id;

  uint32_t *key;
  wl_array_for_each(key, keys) {
    char buf[128];
    xkb_keysym_t sym = xkb_state_key_get_one_sym(context->xkb_state, *key + 8);
    xkb_keysym_get_name(sym, buf, sizeof(buf));
    fprintf(stderr, "sym: %-12s (%d), ", buf, sym);
    xkb_state_key_get_utf8(context->xkb_state, *key + 8, buf, sizeof(buf));
    fprintf(stderr, "utf8: '%s'\n", buf);
  }
}
static void wl_keyboard_key(void *data, struct wl_keyboard *wl_keyboard,
                            uint32_t serial, uint32_t time, uint32_t key,
                            uint32_t state) {
  glps_WaylandContext *context = __get_wl_context(data);
  if (context == NULL)
    return;
  char utf8[128] = "";
  char name[128] = "";
  uint32_t keycode = key + 8;
  xkb_keysym_t sym = xkb_state_key_get_one_sym(context->xkb_state, keycode);
  if (sym == XKB_KEY_NoSymbol)
    return;
  if (xkb_keysym_get_name(sym, name, sizeof(name)) <= 0) {
    name[0] = '\0';
  }
  ssize_t utf8_len =
      xkb_state_key_get_utf8(context->xkb_state, keycode, utf8, sizeof(utf8));
  if (utf8_len <= 0 || utf8[0] == '\0') {
    utf8[0] = '\0';
  }
  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm->callbacks.keyboard_callback != NULL) {
    wm->callbacks.keyboard_callback(
        context->keyboard_window_id,
        state == WL_KEYBOARD_KEY_STATE_PRESSED ? true : false,
        (utf8[0] != '\0' ? utf8 : name), wm->callbacks.keyboard_data);
  }
}

static void wl_keyboard_leave(void *data, struct wl_keyboard *wl_keyboard,
                              uint32_t serial, struct wl_surface *surface) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm->callbacks.keyboard_leave_callback != NULL) {
    wm->callbacks.keyboard_leave_callback(wm->wayland_ctx->keyboard_window_id,
                                          wm->callbacks.keyboard_leave_data);
  }
}
static void wl_keyboard_modifiers(void *data, struct wl_keyboard *wl_keyboard,
                                  uint32_t serial, uint32_t mods_depressed,
                                  uint32_t mods_latched, uint32_t mods_locked,
                                  uint32_t group) {

  glps_WaylandContext *context = __get_wl_context(data);
  if (context == NULL)
    return;
  xkb_state_update_mask(context->xkb_state, mods_depressed, mods_latched,
                        mods_locked, 0, 0, group);
}
static void wl_keyboard_repeat_info(void *data, struct wl_keyboard *wl_keyboard,
                                    int32_t rate, int32_t delay) {}

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

static const struct wl_keyboard_listener wl_keyboard_listener = {
    .keymap = wl_keyboard_keymap,
    .enter = wl_keyboard_enter,
    .leave = wl_keyboard_leave,
    .key = wl_keyboard_key,
    .modifiers = wl_keyboard_modifiers,
    .repeat_info = wl_keyboard_repeat_info,
};

static struct touch_point *get_touch_point(void *data, int32_t id) {

  if (data == NULL)
    return NULL;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_event *touch = &wm->touch_event;
  const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
  int invalid = -1;
  for (size_t i = 0; i < nmemb; ++i) {
    if (touch->points[i].id == id) {
      return &touch->points[i];
    }
    if (invalid == -1 && !touch->points[i].valid) {
      invalid = i;
    }
  }
  if (invalid == -1) {
    return NULL;
  }
  touch->points[invalid].valid = true;
  touch->points[invalid].id = id;
  return &touch->points[invalid];
}

static void wl_touch_down(void *data, struct wl_touch *wl_touch,
                          uint32_t serial, uint32_t time,
                          struct wl_surface *surface, int32_t id, wl_fixed_t x,
                          wl_fixed_t y) {

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  ssize_t window_id = __get_window_id_from_surface(wm, surface);
  if (window_id < 0) {
    LOG_ERROR("Window id is invalid.");
    return;
  }

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_UP;
  point->surface_x = wl_fixed_to_double(x),
  point->surface_y = wl_fixed_to_double(y);
  wm->touch_event.time = time;
  wm->touch_event.serial = serial;

  glps_WaylandContext *context = __get_wl_context(wm);
  if (context == NULL) {
    LOG_ERROR("Couldn't fetch wayland context.");
    return;
  }

  context->touch_window_id = (size_t)window_id;
}

static void wl_touch_up(void *data, struct wl_touch *wl_touch, uint32_t serial,
                        uint32_t time, int32_t id) {

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_UP;
}

static void wl_touch_motion(void *data, struct wl_touch *wl_touch,
                            uint32_t time, int32_t id, wl_fixed_t x,
                            wl_fixed_t y) {

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_MOTION;
  point->surface_x = x, point->surface_y = y;
  wm->touch_event.time = time;
}

static void wl_touch_cancel(void *data, struct wl_touch *wl_touch) {

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  wm->touch_event.event_mask |= TOUCH_EVENT_CANCEL;
}

static void wl_touch_shape(void *data, struct wl_touch *wl_touch, int32_t id,
                           wl_fixed_t major, wl_fixed_t minor) {

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_SHAPE;
  point->major = major, point->minor = minor;
}

static void wl_touch_orientation(void *data, struct wl_touch *wl_touch,
                                 int32_t id, wl_fixed_t orientation) {

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_point *point = get_touch_point(wm, id);
  if (point == NULL) {
    return;
  }
  point->event_mask |= TOUCH_EVENT_ORIENTATION;
  point->orientation = orientation;
}

static void wl_touch_frame(void *data, struct wl_touch *wl_touch) {

  if (data == NULL)
    return;

  glps_WindowManager *wm = (glps_WindowManager *)data;

  struct touch_event *touch = &wm->touch_event;
  const size_t nmemb = sizeof(touch->points) / sizeof(struct touch_point);
  fprintf(stderr, "touch event @ %d:\n", touch->time);

  for (size_t i = 0; i < nmemb; ++i) {
    struct touch_point *point = &touch->points[i];
    if (!point->valid) {
      continue;
    }
    if (wm->callbacks.touch_callback) {
      wm->callbacks.touch_callback(
          touch->window_id,
          touch->points[i].id,                  // id
          wl_fixed_to_double(point->surface_x), // touch_x
          wl_fixed_to_double(point->surface_y), // touch_y
          (point->event_mask & (TOUCH_EVENT_DOWN | TOUCH_EVENT_UP))
              ? true
              : false,                            // state (down/up)
          wl_fixed_to_double(point->major),       // major
          wl_fixed_to_double(point->minor),       // minor
          wl_fixed_to_double(point->orientation), // orientation
          wm->callbacks.touch_data);
    }
    point->valid = false;
  }
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

static const struct wl_touch_listener wl_touch_listener = {
    .down = wl_touch_down,
    .up = wl_touch_up,
    .motion = wl_touch_motion,
    .frame = wl_touch_frame,
    .cancel = wl_touch_cancel,
    .shape = wl_touch_shape,
    .orientation = wl_touch_orientation,
};

static void wl_seat_capabilities(void *data, struct wl_seat *wl_seat,
                                 uint32_t capabilities) {

  glps_WindowManager *context = (glps_WindowManager *)data;
  bool have_pointer = capabilities & WL_SEAT_CAPABILITY_POINTER;
  if (have_pointer && context->wayland_ctx->wl_pointer == NULL) {
    context->wayland_ctx->wl_pointer =
        wl_seat_get_pointer(context->wayland_ctx->wl_seat);
    wl_pointer_add_listener(context->wayland_ctx->wl_pointer,
                            &wl_pointer_listener, data);
  } else if (!have_pointer && context->wayland_ctx->wl_pointer != NULL) {
    wl_pointer_release(context->wayland_ctx->wl_pointer);
    context->wayland_ctx->wl_pointer = NULL;
  }

  bool have_keyboard = capabilities & WL_SEAT_CAPABILITY_KEYBOARD;

  if (have_keyboard && context->wayland_ctx->wl_keyboard == NULL) {
    context->wayland_ctx->wl_keyboard =
        wl_seat_get_keyboard(context->wayland_ctx->wl_seat);
    wl_keyboard_add_listener(context->wayland_ctx->wl_keyboard,
                             &wl_keyboard_listener, data);
  } else if (!have_keyboard && context->wayland_ctx->wl_keyboard != NULL) {
    wl_keyboard_release(context->wayland_ctx->wl_keyboard);
    context->wayland_ctx->wl_keyboard = NULL;
  }

  bool have_touch = capabilities & WL_SEAT_CAPABILITY_TOUCH;

  if (have_touch && context->wayland_ctx->wl_touch == NULL) {
    context->wayland_ctx->wl_touch =
        wl_seat_get_touch(context->wayland_ctx->wl_seat);
    wl_touch_add_listener(context->wayland_ctx->wl_touch, &wl_touch_listener,
                          data);
  } else if (!have_touch && context->wayland_ctx->wl_touch != NULL) {
    wl_touch_release(context->wayland_ctx->wl_touch);
    context->wayland_ctx->wl_touch = NULL;
  }
}

static void wl_seat_name(void *data, struct wl_seat *wl_seat,
                         const char *name) {}

static const struct wl_seat_listener wl_seat_listener = {
    .capabilities = wl_seat_capabilities,
    .name = wl_seat_name,
};

static void data_source_handle_send(void *data, struct wl_data_source *source,
                                    const char *mime_type, int fd) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_WaylandContext *context = NULL;

  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  if ((context = __get_wl_context(wm)) == NULL) {
    LOG_ERROR("Failed to get Wayland context from Window Manager.");
    return;
  }

  if (fd < 0) {
    LOG_ERROR("Invalid file descriptor: %d", fd);
    return;
  }

  LOG_INFO("Copying to clipboard: MIME type=%s, Data Preview=%.20s", mime_type,
           wm->clipboard.buff);

  if (strcmp(mime_type, wm->clipboard.mime_type) == 0) {
    if (write(fd, wm->clipboard.buff, strlen(wm->clipboard.buff)) < 0) {
      LOG_ERROR("Error writing data to clipboard pipe.");
    }
  } else {
    LOG_WARNING("Unsupported MIME type: %s", mime_type);
  }

  if (close(fd) < 0) {
    LOG_ERROR("Error closing file descriptor.");
  }
}

static void data_source_handle_cancelled(void *data,
                                         struct wl_data_source *source) {
  wl_data_source_destroy(source);
}

static void data_source_handle_target(void *data, struct wl_data_source *source,
                                      const char *mime_type) {
  if (mime_type != NULL) {
    printf("Destination would accept MIME type if dropped: %s\n", mime_type);
  } else {
    printf("Destination would reject if dropped\n");
  }
}

static enum wl_data_device_manager_dnd_action last_dnd_action =
    WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE;

static void data_source_handle_action(void *data, struct wl_data_source *source,
                                      uint32_t dnd_action) {
  last_dnd_action = dnd_action;
  switch (dnd_action) {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
    printf("Destination would perform a move action if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY: {
    printf("Destination would perform a copy action if dropped\n");
    break;
  }
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
    printf("Destination would reject the drag if dropped\n");
    break;
  }
}

static void
data_source_handle_dnd_drop_performed(void *data,
                                      struct wl_data_source *source) {
  printf("Drop performed\n");
}

static void data_source_handle_dnd_finished(void *data,
                                            struct wl_data_source *source) {
  switch (last_dnd_action) {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
    printf("Destination has accepted the drop with a move action\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
    printf("Destination has accepted the drop with a copy action\n");
    break;
  }
}

static const struct wl_data_source_listener data_source_listener = {
    .send = data_source_handle_send,
    .cancelled = data_source_handle_cancelled,
    .target = data_source_handle_target,
    .action = data_source_handle_action,
    .dnd_drop_performed = data_source_handle_dnd_drop_performed,
    .dnd_finished = data_source_handle_dnd_finished,
};

static void data_offer_handle_offer(void *data, struct wl_data_offer *offer,
                                    const char *mime_type) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_WaylandContext *context = NULL;

  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  if ((context = __get_wl_context(wm)) == NULL) {
    LOG_ERROR("Failed to get Wayland context from Window Manager.");
    return;
  }

  //  LOG_INFO("Offered MIME type: %s", mime_type);

  if (strcmp(mime_type, "text/plain") == 0) {
    wl_data_offer_accept(offer, context->current_serial, "text/plain");
  }
}
static void data_offer_handle_source_actions(void *data,
                                             struct wl_data_offer *offer,
                                             uint32_t actions) {
  if (actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE) {
    printf("Drag supports the move action\n");
  }
  if (actions & WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY) {
    printf("Drag supports the copy action\n");
  }
}

static void data_device_handle_drop(void *data,
                                    struct wl_data_device *data_device) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_WaylandContext *context = NULL;
  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }

  if ((context = __get_wl_context(wm)) == NULL) {
    LOG_ERROR("Failed to get Wayland context from Window Manager.");
    return;
  }

  if (context->current_drag_offer == NULL) {
    LOG_ERROR("hello");
  }

  assert(context->current_drag_offer != NULL);
  int fds[2];
  pipe(fds);
  wl_data_offer_receive(context->current_drag_offer, "text/plain", fds[1]);
  close(fds[1]);

  wl_display_roundtrip(context->wl_display);

  char buffer[4096];
  ssize_t bytes_read = read(fds[0], buffer, sizeof(buffer));

  if (wm->callbacks.drag_n_drop_callback) {

    wm->callbacks.drag_n_drop_callback(context->mouse_window_id, "text/plain",
                                       buffer, wm->callbacks.drag_n_drop_data);
  }
  close(fds[0]);

  wl_data_offer_finish(context->current_drag_offer);
  wl_data_offer_destroy(context->current_drag_offer);
  context->current_drag_offer = NULL;
}

static void data_offer_handle_action(void *data, struct wl_data_offer *offer,
                                     uint32_t dnd_action) {
  switch (dnd_action) {
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_MOVE:
    // printf("A move action would be performed if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY:
    // printf("A copy action would be performed if dropped\n");
    break;
  case WL_DATA_DEVICE_MANAGER_DND_ACTION_NONE:
    // printf("The drag would be rejected if dropped\n");
    break;
  }
}

static const struct wl_data_offer_listener data_offer_listener = {
    .offer = data_offer_handle_offer,
    .source_actions = data_offer_handle_source_actions,
    .action = data_offer_handle_action,
};

static void data_device_handle_data_offer(void *data,
                                          struct wl_data_device *data_device,
                                          struct wl_data_offer *offer) {
  glps_WindowManager *wm = (glps_WindowManager *)data;

  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL during clipboard selection.");
    return;
  }

  wl_data_offer_add_listener(offer, &data_offer_listener, data);
}
static void data_device_handle_selection(void *data,
                                         struct wl_data_device *data_device,
                                         struct wl_data_offer *offer) {
  glps_WindowManager *wm = (glps_WindowManager *)data;

  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL during clipboard selection.");
    return;
  }

  if (offer == NULL) {
    LOG_INFO("Clipboard is empty.");
    memset(&wm->clipboard, 0, sizeof(wm->clipboard));
    return;
  }

  int fds[2];
  if (pipe(fds) < 0) {
    perror("Failed to create pipe for clipboard data");
    return;
  }

  wl_data_offer_receive(offer, "text/plain", fds[1]);
  close(fds[1]);

  glps_WaylandContext *context = __get_wl_context(wm);
  if (context == NULL) {
    LOG_ERROR("Failed to get Wayland context.");
    close(fds[0]);
    return;
  }

  wl_display_roundtrip(context->wl_display);

  char buf[1024];
  ssize_t n;
  memset(buf, 0, sizeof(buf));
  wm->clipboard.buff[0] = '\0';

  while ((n = read(fds[0], buf, sizeof(buf) - 1)) > 0) {
    buf[n] = '\0';
    strcat(wm->clipboard.buff, buf);
  }

  close(fds[0]);

  if (n < 0) {
    LOG_ERROR("Error reading clipboard data.");
  }

  wl_data_offer_destroy(offer);
}
static void data_device_handle_enter(void *data,
                                     struct wl_data_device *data_device,
                                     uint32_t serial,
                                     struct wl_surface *surface, wl_fixed_t x,
                                     wl_fixed_t y,
                                     struct wl_data_offer *offer) {
  printf("Drag entered surface: %fx%f\n", wl_fixed_to_double(x),
         wl_fixed_to_double(y));
  // Set the current offer
  glps_WaylandContext *ctx = __get_wl_context((glps_WindowManager *)data);
  ctx->current_drag_offer = offer;
  ctx->current_serial = serial;
  wl_data_offer_set_actions(offer, WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY,
                            WL_DATA_DEVICE_MANAGER_DND_ACTION_COPY);
}

static void data_device_handle_motion(void *data,
                                      struct wl_data_device *data_device,
                                      uint32_t time, wl_fixed_t x,
                                      wl_fixed_t y) {
  // This space is intentionally left blank
}

static void data_device_handle_leave(void *data,
                                     struct wl_data_device *data_device) {

  glps_WindowManager *wm = (glps_WindowManager *)data;
  if (wm == NULL) {
    LOG_ERROR("Window Manager is NULL.");
    return;
  }
  glps_WaylandContext *ctx = (glps_WaylandContext *)__get_wl_context(wm);
  if (ctx == NULL) {
    LOG_ERROR("Wayland Context is NULL.");
    return;
  }
  printf("Drag left our surface\n");
  ctx->current_drag_offer = NULL;
}

static const struct wl_data_device_listener data_device_listener = {
    .data_offer = data_device_handle_data_offer,
    .selection = data_device_handle_selection,
    .enter = data_device_handle_enter,
    .motion = data_device_handle_motion,
    .leave = data_device_handle_leave,
    .drop = data_device_handle_drop,

};

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
  struct wl_data_source *source =
      wl_data_device_manager_create_data_source(context->data_dvc_manager);
  wl_data_source_add_listener(source, &data_source_listener, wm);
  wl_data_source_offer(source, "text/plain");
  wl_data_source_offer(source, "text/html");
  wl_data_device_set_selection(context->data_dvc, source,
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

static void handle_global(void *data, struct wl_registry *registry, uint32_t id,
                          const char *interface, uint32_t version) {
  glps_WindowManager *context = (glps_WindowManager *)data;
  glps_WaylandContext *s = (glps_WaylandContext *)context->wayland_ctx;

  LOG_INFO("Attempting to bind interface: %s (id=%u, version=%u)", interface,
           id, version);

  if (strcmp(interface, "wl_compositor") == 0) {
    s->wl_compositor =
        wl_registry_bind(registry, id, &wl_compositor_interface, 1);
    if (!s->wl_compositor) {
      LOG_ERROR("Failed to bind wl_compositor.");
    } else {
      LOG_INFO("Successfully bound wl_compositor.");
    }
  } else if (strcmp(interface, "xdg_wm_base") == 0) {
    s->xdg_wm_base = wl_registry_bind(registry, id, &xdg_wm_base_interface, 1);
    if (!s->xdg_wm_base) {
      LOG_ERROR("Failed to bind xdg_wm_base.");
    } else {
      LOG_INFO("Successfully bound xdg_wm_base.");
    }
  } else if (strcmp(interface, "zxdg_decoration_manager_v1") == 0) {
    s->decoration_manager = wl_registry_bind(
        registry, id, &zxdg_decoration_manager_v1_interface, version);
    if (!s->decoration_manager) {
      LOG_ERROR("Failed to bind zxdg_decoration_manager_v1.");
    } else {
      LOG_INFO("Successfully bound zxdg_decoration_manager_v1.");
    }

  } else if (strcmp(interface, wl_seat_interface.name) == 0) {
    s->wl_seat = wl_registry_bind(registry, id, &wl_seat_interface, version);
    if (s->wl_seat) {
      wl_seat_add_listener(s->wl_seat, &wl_seat_listener, data);
      LOG_INFO("Successfully bound wl_seat and added listener.");
    } else {
      LOG_ERROR("Failed to bind wl_seat.");
    }
  } else if (strcmp(interface, wl_data_device_manager_interface.name) == 0) {
    s->data_dvc_manager =
        wl_registry_bind(registry, id, &wl_data_device_manager_interface, 3);
    if (s->data_dvc_manager) {
      if (s->wl_seat) {
        s->data_dvc = wl_data_device_manager_get_data_device(
            s->data_dvc_manager, s->wl_seat);
        if (s->data_dvc == NULL) {
          LOG_ERROR("Failed to get data device.");
        } else {
          wl_data_device_add_listener(s->data_dvc, &data_device_listener,
                                      context);
        }
      } else {
        LOG_ERROR("Failed to bind data device manager: No wl_seat found.");
      }
    } else {
      LOG_ERROR("Failed to bind wl_data_device_manager_interface.");
    }
  } else {
    LOG_WARNING("Unhandled interface: %s", interface);
  }
}

static void handle_global_remove(void *data, struct wl_registry *registry,
                                 uint32_t name) {}

static const struct wl_registry_listener registry_listener = {
    .global = handle_global,
    .global_remove = handle_global_remove,
};

void glps_display_disconnect(glps_WindowManager *wm) {
  wl_display_disconnect(wm->wayland_ctx->wl_display);
}

void glps_wm_run(glps_WindowManager *wm) {
  while (true) {
    if (wl_display_dispatch(wm->wayland_ctx->wl_display) == -1) {
      fprintf(stderr, "Error in Wayland event dispatch\n");
      break;
    }
  }
}

void glps_wm_update(glps_WindowManager *wm, size_t window_id) {
  int width = 640, height = 480;
  wl_surface_damage(wm->windows[window_id]->wl_surface, 0, 0, width, height);
  wl_surface_commit(wm->windows[window_id]->wl_surface);
}

void glps_wm_swap_interval(glps_WindowManager *wm, unsigned int swap_interval) {
  eglSwapInterval(wm->egl_ctx->dpy, 0);
}

void glps_wm_swap_buffers(glps_WindowManager *wm, size_t window_id) {
  eglSwapBuffers(wm->egl_ctx->dpy, wm->windows[window_id]->egl_surface);
}

static void frame_callback_done(void *data, struct wl_callback *callback,
                                uint32_t time) {
  frame_callback_args *args = (frame_callback_args *)data;
  struct wl_surface *surface = args->wl_surface;

  if (!args)
    exit(EXIT_FAILURE);

  printf("called \n");
  // TODO: render(args->window_id);

  if (callback) {
    wl_callback_destroy(callback);
  }

  if (surface) {
    struct wl_callback *new_callback = wl_surface_frame(surface);
    if (new_callback) {

      wl_callback_add_listener(new_callback, &frame_callback_listener, args);
    }
  }
}

static const struct wl_callback_listener frame_callback_listener = {
    .done = frame_callback_done};

static void handle_toplevel_configure(void *data, struct xdg_toplevel *toplevel,
                                      int32_t width, int32_t height,
                                      struct wl_array *states) {
  glps_WindowManager *wm = (glps_WindowManager *)data;

  ssize_t window_id = __get_window_id_from_xdg_toplevel(wm, toplevel);
  if (window_id < 0) {
    LOG_ERROR("Window ID is invalid.");
    return;
  }
  glps_WaylandWindow *window = wm->windows[window_id];

  if (width != 0 && height != 0) {
    window->properties.height = height;
    window->properties.width = width;
  }
}

static void handle_toplevel_close() { LOG_ERROR("Window closed."); }

struct xdg_toplevel_listener toplevel_listener = {
    .configure = handle_toplevel_configure,
    .close = handle_toplevel_close,
};

static void xdg_surface_configure(void *data, struct xdg_surface *xdg_surface,
                                  uint32_t serial) {
  glps_WindowManager *wm = (glps_WindowManager *)data;

  if (wm == NULL) {
    LOG_ERROR("Couldn't configure XDG Surface. Window Manager is NULL.");
    return;
  }

  glps_WaylandContext *ctx = __get_wl_context(wm);
  if (ctx == NULL) {
    LOG_ERROR("Couldn't configure XDG Surface. Window Manager is NULL.");
    return;
  }

  xdg_surface_ack_configure(xdg_surface, serial);

  ssize_t window_id = __get_window_id_from_xdg_surface(wm, xdg_surface);
  if (window_id < 0) {
    return;
  }

  wm->windows[(size_t)window_id]->serial = serial;
}

static const struct xdg_surface_listener xdg_surface_listener = {
    .configure = xdg_surface_configure,
};

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
    fprintf(stderr, "Failed to initialize EGL\n");
    exit(EXIT_FAILURE);
  }

  printf("EGL initialized successfully (version %d.%d)\n", major, minor);

  if (!eglChooseConfig(wm->egl_ctx->dpy, config_attribs, &wm->egl_ctx->conf, 1,
                       &n) ||
      n != 1) {
    fprintf(stderr, "Failed to choose a valid EGL config\n");
    exit(EXIT_FAILURE);
  }

  if (!eglBindAPI(EGL_OPENGL_API)) {
    fprintf(stderr, "Failed to bind OpenGL API\n");
    exit(EXIT_FAILURE);
  }
  EGLint error = eglGetError();
  if (error != EGL_SUCCESS) {
    fprintf(stderr, "EGL error: %x\n", error);
  }
}
glps_WindowManager *glps_wm_init(void) {
  glps_WindowManager *wm = malloc(sizeof(glps_WindowManager));
  *wm = (glps_WindowManager){0};
  if (!wm) {
    fprintf(stderr, "Failed to allocate memory for glps_WindowManager\n");
    return NULL;
  }

  wm->windows = malloc(sizeof(glps_WaylandWindow *) * 100);
  if (!wm->windows) {
    fprintf(stderr, "Failed to allocate memory for windows array\n");
    free(wm);
    return NULL;
  }

  wm->wayland_ctx = malloc(sizeof(glps_WaylandContext));
  *wm->wayland_ctx = (glps_WaylandContext){0};
  if (!wm->wayland_ctx) {
    fprintf(stderr, "Failed to allocate memory for Wayland context\n");
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
    fprintf(stderr, "Failed to connect to Wayland display\n");
    free(wm->shared_ogl_ctx);
    free(wm->wayland_ctx);
    free(wm->windows);
    free(wm);
    return NULL;
  }

  wm->wayland_ctx->wl_registry =
      wl_display_get_registry(wm->wayland_ctx->wl_display);
  if (!wm->wayland_ctx->wl_registry) {
    fprintf(stderr, "Failed to get Wayland registry\n");
    wl_display_disconnect(wm->wayland_ctx->wl_display);
    free(wm->shared_ogl_ctx);
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
    fprintf(stderr, "xdg_wm_base protocol not supported by compositor\n");
  }

  if (!wm->wayland_ctx->decoration_manager) {
    fprintf(stderr, "xdg-decoration protocol not supported by compositor\n");
  }

  if (!wm->wayland_ctx->wl_compositor || !wm->wayland_ctx->xdg_wm_base) {
    fprintf(stderr, "Failed to retrieve Wayland compositor or xdg_wm_base\n");
    wl_registry_destroy(wm->wayland_ctx->wl_registry);
    wl_display_disconnect(wm->wayland_ctx->wl_display);
    free(wm->shared_ogl_ctx);
    free(wm->wayland_ctx);
    free(wm->windows);
    free(wm);
    return NULL;
  }

  _init_egl(wm);

  return wm;
}

void glps_wm_set_window_ctx_curr(glps_WindowManager *wm, size_t window_id) {
  if (!eglMakeCurrent(wm->egl_ctx->dpy, wm->windows[window_id]->egl_surface,
                      wm->windows[window_id]->egl_surface, wm->egl_ctx->ctx)) {
    EGLint error = eglGetError();
    fprintf(stderr, "eglMakeCurrent failed: 0x%x\\n", error);
    if (error == EGL_BAD_DISPLAY)
      fprintf(stderr, "Invalid EGL display\\n");
    if (error == EGL_BAD_SURFACE)
      fprintf(stderr, "Invalid draw or read surface\\n");
    if (error == EGL_BAD_CONTEXT)
      fprintf(stderr, "Invalid EGL context\\n");
    if (error == EGL_BAD_MATCH)
      fprintf(stderr, "Context or surface attributes mismatch\\n");
    exit(EXIT_FAILURE);
  }
}

size_t glps_wm_window_create(glps_WindowManager *wm, const char *title,
                             int width, int height) {
  glps_WaylandWindow *window = malloc(sizeof(glps_WaylandWindow));
  window->specific_ogl_ctx = malloc(sizeof(glps_WindowOpenGLContext));
  window->wl_surface =
      wl_compositor_create_surface(wm->wayland_ctx->wl_compositor);
  if (!window->wl_surface) {
    fprintf(stderr, "Failed to create wayland surface\n");
    exit(EXIT_FAILURE);
  }

  window->xdg_surface = xdg_wm_base_get_xdg_surface(
      wm->wayland_ctx->xdg_wm_base, window->wl_surface);

  if (!window->xdg_surface) {
    fprintf(stderr, "Failed to create XDG surface\n");
    exit(EXIT_FAILURE);
  }

  if (xdg_surface_add_listener(window->xdg_surface, &xdg_surface_listener,
                               wm) == -1) {
    fprintf(stderr, "Failed to add XDG surface listener\n");
    exit(EXIT_FAILURE);
  }

  window->xdg_toplevel = xdg_surface_get_toplevel(window->xdg_surface);
  if (!window->xdg_toplevel) {
    fprintf(stderr, "Failed to create toplevel\n");
    exit(EXIT_FAILURE);
  }

  xdg_toplevel_set_title(window->xdg_toplevel, title);
  strcpy(window->properties.title, title);
  xdg_toplevel_add_listener(window->xdg_toplevel, &toplevel_listener, wm);
  if (wm->wayland_ctx->decoration_manager) {
    zxdg_toplevel_decoration_v1_set_mode(
        zxdg_decoration_manager_v1_get_toplevel_decoration(
            wm->wayland_ctx->decoration_manager, window->xdg_toplevel),
        ZXDG_TOPLEVEL_DECORATION_V1_MODE_SERVER_SIDE);
  }

  wl_surface_commit(window->wl_surface);

  wl_display_roundtrip(wm->wayland_ctx->wl_display);

  window->egl_window = wl_egl_window_create(window->wl_surface, 640, 480);
  if (!window->egl_window) {
    fprintf(stderr, "Failed to create EGL window\n");
    exit(EXIT_FAILURE);
  }

  window->egl_surface =
      eglCreateWindowSurface(wm->egl_ctx->dpy, wm->egl_ctx->conf,
                             (NativeWindowType)window->egl_window, NULL);
  if (window->egl_surface == EGL_NO_SURFACE) {
    fprintf(stderr, "Failed to create EGL surface\n");
    exit(EXIT_FAILURE);
  }

  wm->windows[wm->window_count] = window;

  if (wm->window_count == 0) {
    _create_ctx(wm);
    glps_wm_set_window_ctx_curr(wm, 0);
    glps_opengl_init(wm);
    glps_opengl_setup_shared(wm);
  }

  glps_opengl_setup_separate(wm, wm->window_count);

  // setup frame callback
  frame_callback_args *frame_args =
      (frame_callback_args *)malloc(sizeof(frame_callback_args));
  struct wl_callback *callback = wl_surface_frame(window->wl_surface);
  frame_args->wl_surface = window->wl_surface;
  frame_args->window_id = wm->window_count;
  wl_callback_add_listener(callback, &frame_callback_listener, frame_args);
  window->frame_arg = frame_args;

  return wm->window_count++;
}

static void _cleanup_wl(glps_WindowManager *wm) {
  for (size_t i = 0; i < wm->window_count; ++i) {
    if (wm->windows[i]) {
      if (wm->windows[i]->wl_surface) {
        wl_surface_destroy(wm->windows[i]->wl_surface);
        wm->windows[i]->wl_surface = NULL;
      }
      if (wm->windows[i]->xdg_surface) {
        xdg_surface_destroy(wm->windows[i]->xdg_surface);
        wm->windows[i]->xdg_surface = NULL;
      }
      if (wm->windows[i]->xdg_toplevel) {
        xdg_toplevel_destroy(wm->windows[i]->xdg_toplevel);
        wm->windows[i]->xdg_toplevel = NULL;
      }

      if (wm->windows[i]->frame_arg) {
        free(wm->windows[i]->frame_arg);
        wm->windows[i]->frame_arg = NULL;
      }

      free(wm->windows[i]);
      wm->windows[i] = NULL;
    }
  }
  free(wm->windows);
  wm->windows = NULL;

  if (wm->wayland_ctx) {
    if (wm->wayland_ctx->wl_seat) {
      wl_seat_destroy(wm->wayland_ctx->wl_seat);
    }
    if (wm->wayland_ctx->xdg_wm_base) {
      xdg_wm_base_destroy(wm->wayland_ctx->xdg_wm_base);
    }
    if (wm->wayland_ctx->decoration_manager) {
      //   zxdg_decoration_manager_v1_destroy(wm->wayland_ctx->decoration_manager);
    }
    if (wm->wayland_ctx->wl_compositor) {
      wl_compositor_destroy(wm->wayland_ctx->wl_compositor);
    }
    if (wm->wayland_ctx->wl_registry) {
      wl_registry_destroy(wm->wayland_ctx->wl_registry);
    }
    glps_display_disconnect(wm);
    free(wm->wayland_ctx);
    wm->wayland_ctx = NULL;
  }
}

static void _cleanup_egl(glps_WindowManager *wm) {
  for (size_t i = 0; i < wm->window_count; ++i) {
    if (wm->windows[i]->egl_surface) {
      eglDestroySurface(wm->egl_ctx->dpy, wm->windows[i]->egl_surface);
      wm->windows[i]->egl_surface = NULL;
    }
    if (wm->windows[i]->egl_window) {
      wl_egl_window_destroy(wm->windows[i]->egl_window);
      wm->windows[i]->egl_window = NULL;
    }
  }

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
  // TODO implement this.
}

void glps_wm_destroy(glps_WindowManager *wm) {
  glps_opengl_cleanup(wm);
  _cleanup_egl(wm);
  _cleanup_wl(wm);
  if (wm) {
    free(wm);
    wm = NULL;
  }
}
