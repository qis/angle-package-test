#include <gl/error.h>
#include <EGL/egl.h>
#include <string>

namespace egl {

// https://www.khronos.org/registry/EGL/sdk/docs/man/html/eglGetError.xhtml

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override {
    return "EGL Error";
  }

  std::string message(int code) const override {
    switch (static_cast<EGLint>(code)) {
    case EGL_SUCCESS: return "Success";
    case EGL_NOT_INITIALIZED: return "Not initialized";
    case EGL_BAD_ACCESS: return "EGL cannot access a requested resource";
    case EGL_BAD_ALLOC: return "EGL failed to allocate resources for the requested operation";
    case EGL_BAD_ATTRIBUTE: return "An unrecognized attribute or attribute value was passed in the attribute list";
    case EGL_BAD_CONTEXT: return "An EGLContext argument does not name a valid EGL rendering context";
    case EGL_BAD_CONFIG: return "An EGLConfig argument does not name a valid EGL frame buffer configuration";
    case EGL_BAD_CURRENT_SURFACE: return "The current surface of the calling thread is no longer valid";
    case EGL_BAD_DISPLAY: return "An EGLDisplay argument does not name a valid EGL display connection";
    case EGL_BAD_SURFACE: return "An EGLSurface argument does not name a valid surface configured for GL rendering";
    case EGL_BAD_MATCH: return "Arguments are inconsistent";
    case EGL_BAD_PARAMETER: return "One or more argument values are invalid";
    case EGL_BAD_NATIVE_PIXMAP: return "A NativePixmapType argument does not refer to a valid native pixmap";
    case EGL_BAD_NATIVE_WINDOW: return "A NativeWindowType argument does not refer to a valid native window";
    case EGL_CONTEXT_LOST: return "A power management event has occurred";
    }
    return "Unknown error code: " + std::to_string(code);
  }
};

const std::error_category& error_category() {
  static const error_category_impl impl;
  return impl;
}

std::error_code error() {
  return { static_cast<int>(eglGetError()), error_category() };
}

std::error_code error(int ev) {
  return { ev, error_category() };
}

}  // namespace egl
