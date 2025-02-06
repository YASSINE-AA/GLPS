#include <GL/gl.h>
#include <stdio.h>
#include <windows.h>

typedef struct {
  char title[64];
  int width;
  int height;
} glps_WindowProperties;

typedef struct {
  HWND hwnd;
  HDC hdc;
  HGLRC hglrc;
  glps_WindowProperties properties;
} glps_WIN32Window;

typedef struct {
  size_t window_count;
  glps_WIN32Window **windows;
} glps_WindowManager;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
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

BOOL SetPixelFormatForOpenGL(HDC hdc) {
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

void win32_window_create(glps_WindowManager *wm,
                         const char *class_name,
                         const char *title, int width, int height) {

                             HINSTANCE hInstance = GetModuleHandle(NULL);

  HWND hwnd =
      CreateWindowEx(0, class_name, title, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT,
                     CW_USEDEFAULT, width, height, NULL, NULL, hInstance, NULL);

  if (hwnd == NULL) {
    MessageBox(NULL, "CreateWindowEx failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return;
  }

  HDC hdc = GetDC(hwnd);

  if (!SetPixelFormatForOpenGL(hdc)) {
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    return;
  }

  HGLRC hglrc = wglCreateContext(hdc);
  if (!hglrc) {
    MessageBox(NULL, "wglCreateContext failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    return;
  }

  wglMakeCurrent(hdc, hglrc);

  glps_WIN32Window *window =
      (glps_WIN32Window *)malloc(sizeof(glps_WIN32Window));
  if (!window) {
    MessageBox(NULL, "Failed to allocate memory for window!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    wglDeleteContext(hglrc);
    ReleaseDC(hwnd, hdc);
    DestroyWindow(hwnd);
    return;
  }

  window->hwnd = hwnd;
  window->hdc = hdc;
  window->hglrc = hglrc;
  snprintf(window->properties.title, sizeof(window->properties.title), "%s",
           title);
  window->properties.width = width;
  window->properties.height = height;

  wm->windows[wm->window_count] = window;
  wm->window_count++;

  SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)wm);
  ShowWindow(hwnd, SW_SHOW);
  UpdateWindow(hwnd);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow) {
  glps_WindowManager wm = {0};
  wm.windows = (glps_WIN32Window **)malloc(sizeof(glps_WIN32Window *) * 10);
  wm.window_count = 0;

  char g_szClassName[] = "glpsWindowClass";

  // Register the window class
  WNDCLASSEX wc = {0};
  wc.cbSize = sizeof(WNDCLASSEX);
  wc.style = 0;
  wc.lpfnWndProc = WndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = 0;
  wc.hInstance = hInstance;
  wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
  wc.hCursor = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = g_szClassName;
  wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

  if (!RegisterClassEx(&wc)) {
    MessageBox(NULL, "Window Registration Failed!", "Error!",
               MB_ICONEXCLAMATION | MB_OK);
    return 0;
  }
  win32_window_create(&wm, hInstance, g_szClassName, nCmdShow, "Window 1", 400,
                      400);
  win32_window_create(&wm, hInstance, g_szClassName, nCmdShow, "Window 2", 400,
                      400);

  MSG msg;
  while (GetMessage(&msg, NULL, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  for (size_t i = 0; i < wm.window_count; i++) {
    free(wm.windows[i]);
  }
  free(wm.windows);

  return (int)msg.wParam;
}
