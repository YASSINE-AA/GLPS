#include <glps_common.h>

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam,
                                LPARAM lParam) {
  glps_WindowManager *wm =
      (glps_WindowManager *)GetWindowLongPtr(hwnd, GWLP_USERDATA);

  switch (msg) {
  case WM_DESTROY:
    if (wm) {

      for (size_t i = 0; i < wm->window_count; i++) {
        if (wm->windows[i]->hwnd == hwnd) {

          wglMakeCurrent(NULL, NULL);
          wglDeleteContext(wm->windows[i]->hglrc);
          ReleaseDC(wm->windows[i]->hwnd, wm->windows[i]->hdc);

          free(wm->windows[i]);

          for (size_t j = i; j < wm->window_count - 1; j++) {
            wm->windows[j] = wm->windows[j + 1];
          }
          wm->window_count--;
          break;
        }
      }

      if (wm->window_count == 0) {
        PostQuitMessage(0);
      }
    }
    break;

  case WM_PAINT: {
    PAINTSTRUCT ps;
    HDC hdc = BeginPaint(hwnd, &ps);

    for (size_t i = 0; i < wm->window_count; i++) {
      if (wm->windows[i]->hwnd == hwnd) {
        wglMakeCurrent(wm->windows[i]->hdc, wm->windows[i]->hglrc);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        SwapBuffers(wm->windows[i]->hdc);
        break;
      }
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

  win32_window->hglrc = wglCreateContext(win32_window->hdc);
  if (!win32_window->hglrc) {
    MessageBox(NULL, "wglCreateContext failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    ReleaseDC(win32_window->hwnd, win32_window->hdc);
    DestroyWindow(win32_window->hwnd);
    return -1;
  }

  wglMakeCurrent(win32_window->hdc, win32_window->hglrc);

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
