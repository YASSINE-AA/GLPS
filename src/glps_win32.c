#include <glps_common.h>

ssize_t __get_window_id_from_hwnd(glps_WindowManager *wm, HWND hwnd) {
  if (wm == NULL) {
    return -1;
  }
  for (size_t i = 0; i < wm->window_count; ++i) {
    if (wm->windows[i]->hwnd == hwnd) {
      return i;
    }
  }

  return -1;
}

void __get_special_key_name(UINT wParam, char *char_value, size_t size) {
  switch (wParam) {
  case VK_ESCAPE:
    strncpy(char_value, "Escape", size);
    break;
  case VK_BACK:
    strncpy(char_value, "Backspace", size);
    break;
  case VK_RETURN:
    strncpy(char_value, "Enter", size);
    break;
  case VK_TAB:
    strncpy(char_value, "Tab", size);
    break;
  case VK_DELETE:
    strncpy(char_value, "Delete", size);
    break;
  case VK_INSERT:
    strncpy(char_value, "Insert", size);
    break;
  case VK_HOME:
    strncpy(char_value, "Home", size);
    break;
  case VK_END:
    strncpy(char_value, "End", size);
    break;
  case VK_PRIOR:
    strncpy(char_value, "PageUp", size);
    break;
  case VK_NEXT:
    strncpy(char_value, "PageDown", size);
    break;
  case VK_LEFT:
    strncpy(char_value, "ArrowLeft", size);
    break;
  case VK_RIGHT:
    strncpy(char_value, "ArrowRight", size);
    break;
  case VK_UP:
    strncpy(char_value, "ArrowUp", size);
    break;
  case VK_DOWN:
    strncpy(char_value, "ArrowDown", size);
    break;
  case VK_F1:
    strncpy(char_value, "F1", size);
    break;
  case VK_F2:
    strncpy(char_value, "F2", size);
    break;
  case VK_F3:
    strncpy(char_value, "F3", size);
    break;
  case VK_F4:
    strncpy(char_value, "F4", size);
    break;
  case VK_F5:
    strncpy(char_value, "F5", size);
    break;
  case VK_F6:
    strncpy(char_value, "F6", size);
    break;
  case VK_F7:
    strncpy(char_value, "F7", size);
    break;
  case VK_F8:
    strncpy(char_value, "F8", size);
    break;
  case VK_F9:
    strncpy(char_value, "F9", size);
    break;
  case VK_F10:
    strncpy(char_value, "F10", size);
    break;
  case VK_F11:
    strncpy(char_value, "F11", size);
    break;
  case VK_F12:
    strncpy(char_value, "F12", size);
    break;
  default:
    char_value[0] = '\0';
  }
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                LPARAM lParam) {
  glps_WindowManager *wm =
      (glps_WindowManager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);
  ssize_t window_id = __get_window_id_from_hwnd(wm, hwnd);
  POINT p = {.x = -1, .y = -1};
  static bool key_states[256] = {false};

  switch (msg) {
  case WM_DESTROY:
    if (window_id < 0 || wm == NULL) {
      break;
    }

    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(wm->win32_ctx->hglrc);
    ReleaseDC(wm->windows[window_id]->hwnd, wm->windows[window_id]->hdc);

    free(wm->windows[window_id]);

    for (size_t j = window_id; j < wm->window_count - 1; j++) {
      wm->windows[j] = wm->windows[j + 1];
    }
    wm->window_count--;

    if (wm->window_count == 0) {
      PostQuitMessage(0);
    }
    break;

  /* =========== Mouse Input ============ */
  case WM_MOUSEMOVE:
    if (window_id < 0 || wm == NULL) {
      break;
    }
    static bool is_mouse_in_window = false;
    GetCursorPos(&p);
    ScreenToClient(hwnd, &p);

    if (!is_mouse_in_window) {
      is_mouse_in_window = true;

      if (wm->callbacks.mouse_enter_callback) {
        wm->callbacks.mouse_enter_callback(window_id, (double)p.x, (double)p.y,
                                           wm->callbacks.mouse_enter_data);
      }

      TRACKMOUSEEVENT tme;
      tme.cbSize = sizeof(TRACKMOUSEEVENT);
      tme.dwFlags = TME_LEAVE;
      tme.hwndTrack = hwnd;
      TrackMouseEvent(&tme);
    }

    if (wm->callbacks.mouse_move_callback) {
      wm->callbacks.mouse_move_callback(window_id, (double)p.x, (double)p.y,
                                        wm->callbacks.mouse_move_data);
    }
    break;

  case WM_MOUSELEAVE:
    is_mouse_in_window = false;

    if (wm && wm->callbacks.mouse_leave_callback) {
      wm->callbacks.mouse_leave_callback(window_id,
                                         wm->callbacks.mouse_leave_data);
    }
    break;

  case WM_LBUTTONDOWN:
    if (window_id < 0 || wm == NULL) {
      break;
    }

    if (wm->callbacks.mouse_click_callback) {
      wm->callbacks.mouse_click_callback(window_id, true,
                                         wm->callbacks.mouse_click_data);
    }
    break;

  case WM_LBUTTONUP:
    if (window_id < 0 || wm == NULL) {
      break;
    }

    if (wm->callbacks.mouse_click_callback) {
      wm->callbacks.mouse_click_callback(window_id, false,
                                         wm->callbacks.mouse_click_data);
    }
    break;

  case WM_MOUSEWHEEL:
    if (window_id < 0 || wm == NULL) {
      break;
    }
    double delta = (double)(GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA);
    DWORD extra_info = GetMessageExtraInfo();

    // TODO: Improve this to have wider source support.
    GLPS_SCROLL_SOURCE source =
        extra_info == 0 ? GLPS_SCROLL_SOURCE_WHEEL : GLPS_SCROLL_SOURCE_FINGER;

    if (wm->callbacks.mouse_scroll_callback) {
      // TODO: impl discrete and is_stopped
      wm->callbacks.mouse_scroll_callback(window_id, GLPS_SCROLL_V_AXIS, source,
                                          delta, -1, false,
                                          wm->callbacks.mouse_scroll_data);
    }
    break;

  /* ======== Keyboard Input ========= */
  case WM_KEYDOWN:

    if (window_id < 0 || wm == NULL) {
      break;
    }

    if (!(lParam & 0x40000000)) { // Prevent auto-repeat
      if (wParam < 256 && !key_states[wParam]) {
        key_states[wParam] = true;

        if (wm->callbacks.keyboard_callback) {
          char key_name[32] = {0};
          GetKeyNameTextA(lParam, key_name, sizeof(key_name));

          BYTE keyboardState[256];
          GetKeyboardState(keyboardState);

          WCHAR unicodeChar = 0;
          char char_value[32] = {0};

          if (ToUnicode(wParam, (lParam >> 16) & 0xFF, keyboardState,
                        &unicodeChar, 1, 0) == 1) {
            WideCharToMultiByte(CP_UTF8, 0, &unicodeChar, 1, char_value,
                                sizeof(char_value), NULL, NULL);
          }

          if (char_value[0] == '\0') {
            __get_special_key_name(wParam, char_value, sizeof(char_value));
            if (char_value[0] == '\0')
              strncpy(char_value, key_name, sizeof(char_value) - 1);
          }

          wm->callbacks.keyboard_callback(window_id, true, char_value,
                                          wm->callbacks.keyboard_data);
        }
      }
    }
    break;

  case WM_KEYUP:

    if (window_id < 0 || wm == NULL) {
      break;
    }

    if (wParam < 256) {
      key_states[wParam] = false;

      if (wm->callbacks.keyboard_callback) {
        char key_name[32] = {0};
        GetKeyNameTextA(lParam, key_name, sizeof(key_name));

        BYTE keyboardState[256];
        GetKeyboardState(keyboardState);

        WCHAR unicodeChar = 0;
        char char_value[32] = {0};

        if (ToUnicode(wParam, (lParam >> 16) & 0xFF, keyboardState,
                      &unicodeChar, 1, 0) == 1) {
          WideCharToMultiByte(CP_UTF8, 0, &unicodeChar, 1, char_value,
                              sizeof(char_value), NULL, NULL);
        }

        if (char_value[0] == '\0') {
          __get_special_key_name(wParam, char_value, sizeof(char_value));
          if (char_value[0] == '\0')
            strncpy(char_value, key_name, sizeof(char_value) - 1);
        }

        wm->callbacks.keyboard_callback(window_id, false, char_value,
                                        wm->callbacks.keyboard_data);
      }
    }
    break;

    /* ======= Window focus ======= */
  case WM_SETFOCUS:

    if (window_id < 0 || wm == NULL) {
      break;
    }

    if (wm->callbacks.keyboard_enter_callback) {
      wm->callbacks.keyboard_enter_callback(window_id,
                                            wm->callbacks.keyboard_enter_data);
    }

    break;

  case WM_KILLFOCUS:

    if (window_id < 0 || wm == NULL) {
      break;
    }

    if (wm->callbacks.keyboard_leave_callback) {
      wm->callbacks.keyboard_leave_callback(window_id,
                                            wm->callbacks.keyboard_leave_data);
    }

    break;

    /* ======== Rendering ========= */
  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    if (window_id < 0 || wm == NULL) {
      EndPaint(hwnd, &ps);
      break;
    }

    if (wm->callbacks.window_frame_update_callback) {
      wm->callbacks.window_frame_update_callback(
          (size_t)window_id, wm->callbacks.window_frame_update_data);
    }

    EndPaint(hwnd, &ps);
    break;
  }

  default:
    return DefWindowProc(hwnd, msg, wParam, lParam);
  }
  return 0;
}
static void __init_window_class(glps_WindowManager *wm,
                                const char *class_name) {
  HINSTANCE hInstance = GetModuleHandle(NULL);

  wm->wc = (WNDCLASSEX){0};
  wm->wc.cbSize = sizeof(WNDCLASSEX);
  wm->wc.style = 0;
  wm->wc.lpfnWndProc = WndProc;
  wm->wc.cbClsExtra = 0;
  wm->wc.cbWndExtra = 0;
  wm->wc.hInstance = hInstance;
  wm->wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wm->wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wm->wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wm->wc.lpszMenuName = NULL;
  wm->wc.lpszClassName = class_name;
  wm->wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  if (!RegisterClassEx(&wm->wc)) {
    MessageBox(NULL, "Window Registration Failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return;
  }
}

void glps_win32_init(glps_WindowManager *wm) {
  __init_window_class(wm, "glpsWindowClass");

  wm->windows = malloc(sizeof(glps_Win32Window *) * MAX_WINDOWS);
  if (!wm->windows) {
    LOG_ERROR("Failed to allocate memory for windows array");
    free(wm);
    return;
  }

  wm->win32_ctx = malloc(sizeof(glps_Win32Context));
  *wm->win32_ctx = (glps_Win32Context){0};
  if (!wm->win32_ctx) {
    LOG_ERROR("Failed to allocate memory for WIN32 context");
    free(wm->windows);
    free(wm);
    return;
  }

  wm->window_count = 0;
}

static BOOL SetPixelFormatForOpenGL(HDC hdc) {
  PIXELFORMATDESCRIPTOR pfd = {sizeof(PIXELFORMATDESCRIPTOR),
                               1,
                               PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
                                   PFD_DOUBLEBUFFER,
                               PFD_TYPE_RGBA,
                               32,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               0,
                               24,
                               8,
                               0,
                               PFD_MAIN_PLANE,
                               0,
                               0,
                               0,
                               0};

  int pixelFormat = ChoosePixelFormat(hdc, &pfd);
  if (pixelFormat == 0) {
    MessageBox(NULL, "ChoosePixelFormat failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return FALSE;
  }

  if (!SetPixelFormat(hdc, pixelFormat, &pfd)) {
    MessageBox(NULL, "SetPixelFormat failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return FALSE;
  }

  return TRUE;
}

ssize_t glps_win32_window_create(glps_WindowManager *wm, const char *title,
                                 int width, int height) {
  HINSTANCE hInstance = GetModuleHandle(NULL);

  glps_Win32Window *win32_window =
      (glps_Win32Window *)malloc(sizeof(glps_Win32Window));

  if (win32_window == NULL) {
    MessageBox(NULL, "Win32 Window allocation failed", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return -1;
  }

  win32_window->hwnd = CreateWindowEx(
      0, wm->wc.lpszClassName, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
      CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

  if (win32_window->hwnd == NULL) {
    MessageBox(NULL, "CreateWindowEx failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return -1;
  }

  win32_window->hdc = GetDC(win32_window->hwnd);

  if (!SetPixelFormatForOpenGL(win32_window->hdc)) {
    ReleaseDC(win32_window->hwnd, win32_window->hdc);
    DestroyWindow(win32_window->hwnd);
    return -1;
  }
  if (wm->window_count == 0) {
    wm->win32_ctx->hglrc = wglCreateContext(win32_window->hdc);
    if (!wm->win32_ctx->hglrc) {
      MessageBox(NULL, "wglCreateContext failed!", "Error!",
                 MB_ICONEXCLAMATION | MB_OK);
      ReleaseDC(win32_window->hwnd, win32_window->hdc);
      DestroyWindow(win32_window->hwnd);
      return -1;
    }
  }

  wglMakeCurrent(win32_window->hdc, wm->win32_ctx->hglrc);

  snprintf(win32_window->properties.title,
           sizeof(win32_window->properties.title), "%s", title);

  win32_window->properties.width = width;
  win32_window->properties.height = height;
  wm->windows[wm->window_count] = win32_window;

  SetWindowLongPtr(win32_window->hwnd, GWLP_USERDATA, (LONG_PTR)wm);
  ShowWindow(win32_window->hwnd, SW_SHOW);
  UpdateWindow(win32_window->hwnd);
  return wm->window_count++;
}

void glps_win32_destroy(glps_WindowManager *wm) {
  for (size_t i = 0; i < wm->window_count; ++i) {
    if (wm->windows[i] != NULL) {
      free(wm->windows[i]);
      wm->windows[i] = NULL;
    }
  }

  if (wm->windows != NULL) {
    free(wm->windows);
    wm->windows = NULL;
  }

  if (wm != NULL) {
    free(wm);
    wm = NULL;
  }
}

HDC glps_win32_get_window_hdc(glps_WindowManager *wm, size_t window_id) {
  return wm->windows[window_id]->hdc;
}
