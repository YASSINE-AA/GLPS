#include <GL/gl.h>
#include <glps_common.h>
#include <windows.h>

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

void win32_init_window_class(glps_WindowManager *wm, const char *class_name) {

  wm->wc = {0};
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
    return 0;
  }
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

void win32_window_create(glps_WindowManager *wm, HINSTANCE hInstance,
                         const char *class_name, int nCmdShow,
                         const char *title, int width, int height) {

  glps_Win32Window *win32_window =
      (glps_Win32Window *)malloc(sizeof(glps_Win32Window));

  if (win32_window == NULL) {
    MessageBox(NULL, "Win32 Window allocation failed", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return;
  }

  win32_window->hwnd =
      CreateWindowEx(0, class_name, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                     CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

  if (win32_window->hwnd == NULL) {
    MessageBox(NULL, "CreateWindowEx failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return;
  }

  win32_window->hdc = GetDC(hwnd);

  if (!SetPixelFormatForOpenGL(win32_window->hdc)) {
    ReleaseDC(win32_window->hwnd, win32_window->hdc);
    DestroyWindow(win32_window->hwnd);
    return;
  }

  win32_window->hglrc = wglCreateContext(win32_window->hdc);
  if (!win32_window->hglrc) {
    MessageBox(NULL, "wglCreateContext failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    ReleaseDC(win32_window->hwnd, win32_window->hdc);
    DestroyWindow(hwnd);
    return;
  }

  wglMakeCurrent(win32_window->hdc, win32_window->hglrc);

  snprintf(window->properties.title, sizeof(window->properties.title), "%s",
           title);

  window->properties.width = width;
  window->properties.height = height;
  wm->windows[wm->window_count] = window;
  wm->window_count++;

  SetWindowLongPtr(win32_window->hwnd, GWLP_USERDATA, (LONG_PTR)wm);
  ShowWindow(win32_window->hwnd, nCmdShow);
  UpdateWindow(win32_window->hwnd);
}
