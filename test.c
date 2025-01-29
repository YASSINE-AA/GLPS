/*
 Copyright (c) 2025 Yassine Ahmed Ali

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "include/glps_common.h"
#include "include/glps_opengl.h"
#include "include/glps_window_manager.h"
#include "internal/utils/logger/pico_logger.h"
glps_WindowManager *wm = NULL;
void mouse_leave_callback(size_t window_id, void *data) {
  LOG_INFO("Mouse left.");
}

void mouse_scroll_callback(size_t window_id, GLPS_SCROLL_AXES axe,
                           GLPS_SCROLL_SOURCE source, double value,
                           int discrete, bool is_stopped, void *data) {

  LOG_INFO("%s scroll via %s with value %.2lf discrete %d stopped %d",
           axe == GLPS_SCROLL_H_AXIS ? "horizontal" : "vertical",
           source == GLPS_SCROLL_SOURCE_FINGER ? "finger" : "idk", value,
           discrete, is_stopped);
}

void drag_n_drop_callback(size_t origin_window_id, char *mime, char *buff,
                          void *data) {
  LOG_INFO("Origin window id: %ld, Mime: %s, Buffer: %s", origin_window_id,
           mime, buff);
}

void mouse_enter_callback(size_t window_id, double mouse_x, double mouse_y,
                          void *data) {
  LOG_INFO("Mouse entered at x: %lf y:%lf", mouse_x, mouse_y);
}

void mouse_click_callback(size_t window_id, bool state, void *data) {
  if (data == NULL) {
    LOG_ERROR("null");
  } else {

    if (state == true)
      glps_wm_attach_to_clipboard((glps_WindowManager *)data, "text/plain",
                                  "dhiee");
  }

  LOG_INFO("Window %ld Mouse %s", window_id, state ? "pressed" : "released");
}

void mouse_move_callback(size_t window_id, double mouse_x, double mouse_y,
                         void *data) {
  LOG_INFO("x: %lf y:%lf", mouse_x, mouse_y);
}

void keyboard_enter_callback() { LOG_INFO("keyboard entered."); }

void keyboard_callback(size_t window_id, bool state, const char *value,
                       void *data) {
  LOG_INFO("window %ld state: %d value:%s", window_id, state, value);
  char buff[1024];
  glps_wm_get_from_clipboard(wm, buff, 1024);
  LOG_INFO("Clipboard content is: %s", buff);
}

void keyboard_leave_callback(size_t window_id, void *data) {
  LOG_INFO("keyboard left.");
}

void render_frame(glps_WindowManager *wm, size_t window_id) {
  glps_wm_set_window_ctx_curr(wm, window_id);
  int width, height;
  glps_wm_window_get_dimensions(wm, window_id, &width, &height);
  glViewport(0, 0, width, height);
  glps_clear(wm);
  glps_opengl_set_text_projection(wm, window_id);
  glps_opengl_fill_rectangle(wm, window_id, 10, 110, 100, 100, 0xFF0000);
  glps_opengl_draw_text(wm, window_id, 20, 50, "test", 0.3f, 0x000000);
  glps_wm_swap_buffers(wm, window_id);
  glps_wm_update(wm, window_id);
}

void window_resize_callback(size_t window_id, int width, int height,
                            void *data) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  render_frame(wm, window_id);
}

void window_close_callback(size_t window_id, void *data) {
  glps_WindowManager *wm = (glps_WindowManager *)data;
  glps_wm_window_destroy(wm, window_id);
}

int main(int argc, char *argv[]) {

  wm = glps_wm_init();

  glps_wm_window_create(wm, "test window", 640, 480);
  glps_wm_window_create(wm, "test window 2", 800, 800);

  /* ================== CALLBACK SETUPS ================== */
  glps_wm_set_keyboard_callback(wm, keyboard_callback, (void *)wm);
  glps_wm_set_mouse_move_callback(wm, mouse_move_callback, (void *)wm);
  glps_wm_set_mouse_click_callback(wm, mouse_click_callback, (void *)wm);
  glps_wm_set_mouse_enter_callback(wm, mouse_enter_callback, (void *)wm);
  glps_wm_set_mouse_leave_callback(wm, mouse_leave_callback, (void *)wm);
  glps_wm_set_keyboard_leave_callback(wm, keyboard_leave_callback, (void *)wm);
  glps_wm_set_keyboard_enter_callback(wm, keyboard_enter_callback, (void *)wm);
  glps_wm_set_scroll_callback(wm, mouse_scroll_callback, (void *)wm);
  glps_wm_window_set_resize_callback(wm, window_resize_callback, (void *)wm);
  glps_wm_window_set_close_callback(wm, window_close_callback, (void *)wm);
  /* ====================================================== */

  glps_ft_init(wm, "./roboto.ttf");

  for (size_t i = 0; i < wm->window_count; ++i) {
    render_frame(wm, i);
  }

  // glps_wm_start_drag_n_drop(wm, 0, drag_n_drop_callback, NULL);

  glps_wm_run(wm);

  glps_wm_destroy(wm);

  return 0;
}
