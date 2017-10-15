#include "window.h"
#include <config.h>
#include <array>
#include <stdexcept>

#ifdef WIN32
#include <windows.h>

class window::impl {
public:
  impl(window* window, int argc, char* argv[]) noexcept : window_(window) {
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = [](HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) -> LRESULT {
      auto self = reinterpret_cast<impl*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
      if (msg == WM_CREATE) {
        self = reinterpret_cast<impl*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
        SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(self));
      } else if (msg == WM_DESTROY) {
        SetWindowLongPtr(hwnd, GWLP_USERDATA, 0);
      }
      return self ? self->handle(hwnd, msg, wparam, lparam) : DefWindowProc(hwnd, msg, wparam, lparam);
    };
    wc.hInstance = hinstance_;
    wc.hIcon = wc.hIconSm = LoadIcon(hinstance_, MAKEINTRESOURCE(101));
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
    wc.lpszClassName = name();
    RegisterClassEx(&wc);
  }

  ~impl() {
    UnregisterClass(name(), hinstance_);
  }

  int run() noexcept {
    const auto es = WS_EX_APPWINDOW;
    const auto ws = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    if (!CreateWindowEx(es, name(), name(), ws, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, nullptr, nullptr, hinstance_, this)) {
      error("Could not create main application window.");
    }
    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    return static_cast<int>(msg.wParam);
  }

  void show(bool show) noexcept {
    ShowWindow(hwnd_, show ? SW_SHOW : SW_HIDE);
  }

  void error(const char* msg) noexcept {
    std::array<wchar_t, 1024> str;
    MultiByteToWideChar(CP_UTF8, 0, msg, -1, str.data(), static_cast<int>(str.size() - 1));
    MessageBox(hwnd_, str.data(), name(), MB_OK | MB_ICONERROR | MB_SETFOREGROUND);
    PostQuitMessage(1);
  }

  void on_create() {
    // Get window monitor.
    auto monitor = MonitorFromWindow(hwnd_, MONITOR_DEFAULTTONULL);
    if (!monitor) {
      throw std::runtime_error("Could not get monitor handle.");
    }

    // Get monitor dpi.
    auto shcore = LoadLibrary(L"shcore.dll");
    if (!shcore) {
      throw std::runtime_error("Could not load shcore.dll.");
    }
    typedef HRESULT(WINAPI *GetDpiForMonitorProc)(HMONITOR, int, UINT*, UINT*);
    auto GetDpiForMonitor = reinterpret_cast<GetDpiForMonitorProc>(GetProcAddress(shcore, "GetDpiForMonitor"));
    if (!GetDpiForMonitor) {
      throw std::runtime_error("Could not get address of GetDpiForMonitor.");
    }
    UINT xdpi = 0;
    UINT ydpi = 0;
    if (SUCCEEDED(GetDpiForMonitor(monitor, 0, &xdpi, &ydpi))) {
      dpi_ = static_cast<GLint>(std::max(1U, ydpi));
    }

    // Center window.
    MONITORINFO mi = {};
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfo(monitor, &mi)) {
      RECT rc = {};
      if (GetWindowRect(hwnd_, &rc)) {
        auto x = ((mi.rcWork.right - mi.rcWork.left) - (rc.right - rc.left)) / 2;
        auto y = ((mi.rcWork.bottom - mi.rcWork.top) - (rc.bottom - rc.top)) / 2;
        SetWindowPos(hwnd_, nullptr, x, y, 0, 0, SWP_NOACTIVATE | SWP_NOSIZE);
      }
    }

    // Get client size.
    RECT rc = {};
    GetClientRect(hwnd_, &rc);
    cx_ = static_cast<GLsizei>(std::max(1L, rc.right - rc.left));
    cy_ = static_cast<GLsizei>(std::max(1L, rc.bottom - rc.top));

    // Notify client.
    window_->on_create(cx_, cy_, dpi_);
  }

  void on_destroy() {
    // Notify client.
    window_->on_destroy();

    // Release display handle.
    ReleaseDC(hwnd_, hdc_);
    hdc_ = {};

    // Show last error message.
    if (exception_) {
      try {
        std::rethrow_exception(exception_);
      }
      catch (const std::exception& e) {
        error(e.what());
      }
    }

    // Stop main message loop.
    PostQuitMessage(0);
  }

  void on_paint() {
    if (!exception_) {
      try {
        window_->on_render();
      }
      catch (...) {
        exception_ = std::current_exception();
        PostMessage(hwnd_, WM_CLOSE, 0, 0);
      }
    } else {
      PAINTSTRUCT ps = {};
      auto hdc = BeginPaint(hwnd_, &ps);
      RECT rc = { 0, 0, cx_, cy_ };
      FillRect(hdc, &rc, reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1));
      EndPaint(hwnd_, &ps);
    }
  }

  void on_size(int cx, int cy) {
    cx_ = static_cast<GLsizei>(std::max(1, cx));
    cy_ = static_cast<GLsizei>(std::max(1, cy));
    window_->on_resize(cx_, cy_, dpi_);
  }

  void on_dpi(int dpi, LPCRECT rc) {
    SetWindowPos(hwnd_, nullptr, rc->left, rc->top, rc->right - rc->left, rc->bottom - rc->top, SWP_NOZORDER | SWP_NOACTIVATE);
    dpi_ = static_cast<GLint>(std::max(1, dpi));
    window_->on_resize(cx_, cy_, dpi_);
  }

  EGLNativeWindowType native_window() const {
    return hwnd_;
  }

  EGLNativeDisplayType nateive_display() const {
    return hdc_;
  }

  static const wchar_t* name() noexcept {
    return TEXT(PROJECT);
  }

private:
  LRESULT handle(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) noexcept {
    try {
      switch (msg) {
      case WM_CREATE:
        hwnd_ = hwnd;
        on_create();
        return 0;
      case WM_DESTROY:
        on_destroy();
        hwnd_ = {};
        return 0;
      case WM_PAINT:
        on_paint();
        return 0;
      case WM_SIZE:
        on_size(LOWORD(lparam), HIWORD(lparam));
        return 0;
      case WM_ERASEBKGND:
        return 1;
      case WM_GETMINMAXINFO:
        if (auto mmi = reinterpret_cast<LPMINMAXINFO>(lparam)) {
          mmi->ptMinTrackSize.x = 300;
          mmi->ptMinTrackSize.y = 250;
        }
        return 0;
      case WM_DPICHANGED:
        on_dpi(HIWORD(wparam), reinterpret_cast<LPCRECT>(lparam));
        return 0;
      }
    }
    catch (const std::exception& e) {
      error(e.what());
    }
    return DefWindowProc(hwnd, msg, wparam, lparam);
  }

  window* window_ = nullptr;
  HINSTANCE hinstance_ = GetModuleHandle(nullptr);
  HWND hwnd_ = {};
  HDC hdc_ = {};

  GLsizei cx_ = 1;
  GLsizei cy_ = 1;
  GLint dpi_ = 96;

  std::exception_ptr exception_;
};

#endif

window::window(int argc, char* argv[]) : impl_(std::make_unique<impl>(this, argc, argv)) {
}

window::~window() = default;

int window::run() noexcept {
  return impl_->run();
}

void window::show(bool show) noexcept {
  impl_->show(show);
}

EGLNativeWindowType window::native_window() const {
  return impl_->native_window();
}

EGLNativeDisplayType window::nateive_display() const {
  return impl_->nateive_display();
}
