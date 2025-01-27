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
void mouse_leave_callback(void *data) { LOG_INFO("Mouse left."); }

void mouse_scroll_callback(GLPS_SCROLL_AXES axe, GLPS_SCROLL_SOURCE source,
                           double value, int discrete, bool is_stopped,
                           void *data) {

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

void mouse_enter_callback(double mouse_x, double mouse_y, void *data) {
  LOG_INFO("Mouse entered at x: %lf y:%lf", mouse_x, mouse_y);
}

void mouse_click_callback(bool state, void *data) {
  if (data == NULL) {
    LOG_ERROR("null");
  } else {

    if (state == true)
      glps_wm_attach_to_clipboard((glps_WindowManager *)data, "text/plain",
                                  "dhiee");
  }

  LOG_INFO("Mouse %s", state ? "pressed" : "released");
}

void mouse_move_callback(double mouse_x, double mouse_y, void *data) {
  LOG_INFO("x: %lf y:%lf", mouse_x, mouse_y);
}

void keyboard_enter_callback() { LOG_INFO("keyboard entered."); }

void keyboard_callback(bool state, const char *value, void *data) {
  LOG_INFO("state: %d value:%s", state, value);
  char buff[1024];
  glps_wm_get_from_clipboard(wm, buff, 1024);
  LOG_INFO("Clipboard content is: %s", buff);
}

void keyboard_leave_callback(void *data) { LOG_INFO("keyboard left."); }

int main(int argc, char *argv[]) {

  wm = glps_wm_init();

  glps_wm_window_create(wm, "test window", 0, 0);
  glps_wm_window_create(wm, "test window", 0, 0);

  /* ================== CALLBACK SETUPS ================== */
  glps_wm_set_keyboard_callback(wm, keyboard_callback, (void *)wm);
  glps_wm_set_mouse_move_callback(wm, mouse_move_callback, (void *)wm);
  glps_wm_set_mouse_click_callback(wm, mouse_click_callback, (void *)wm);
  glps_wm_set_mouse_enter_callback(wm, mouse_enter_callback, (void *)wm);
  glps_wm_set_mouse_leave_callback(wm, mouse_leave_callback, (void *)wm);
  glps_wm_set_keyboard_leave_callback(wm, keyboard_leave_callback, (void *)wm);
  glps_wm_set_keyboard_enter_callback(wm, keyboard_enter_callback, (void *)wm);
  glps_wm_set_scroll_callback(wm, mouse_scroll_callback, (void *)wm);
  /* ====================================================== */

  glps_ft_init(wm, "./roboto.ttf");
  for (size_t i = 0; i < wm->window_count; ++i) {
    glps_wm_set_window_ctx_curr(wm, i);
    glps_clear(wm);
    glps_opengl_set_text_projection(wm, i);
    glps_opengl_fill_rectangle(wm, i, 10, 110, 100, 100, 0xFF0000);
    // glps_opengl_draw_window_borders(wm, i, 0xD3D3D3);
    glps_opengl_draw_text(wm, i, 20, 50, "test", 0.3f, 0x000000);
    glps_wm_swap_buffers(wm, i);
    glps_wm_update(wm, i);
  }
  glps_wm_start_drag_n_drop(wm, 0, drag_n_drop_callback, NULL);

  glps_wm_run(wm);

  glps_wm_destroy(wm);

  return 0;
}
