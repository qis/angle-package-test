#include <gl/error.h>
#include <GLES3/gl3.h>
#include <string>

namespace gl {

// https://www.khronos.org/registry/OpenGL-Refpages/es3.0/html/glGetError.xhtml

class error_category_impl : public std::error_category {
public:
  const char* name() const noexcept override {
    return "OpenGL Error";
  }

  std::string message(int code) const override {
    switch (static_cast<GLenum>(code)) {
    case GL_NO_ERROR: "Success";
    case GL_INVALID_ENUM: "Invalid enumerated argument";
    case GL_INVALID_VALUE: "Invalid numeric argument";
    case GL_INVALID_OPERATION: "Invalid operation";
    case GL_INVALID_FRAMEBUFFER_OPERATION: "Invalid framebuffer operation";
    case GL_OUT_OF_MEMORY: "Out of memory";
    }
    return "Unknown error code: " + std::to_string(code);
  }
};

const std::error_category& error_category() {
  static const error_category_impl impl;
  return impl;
}

std::error_code error() {
  return { static_cast<int>(glGetError()), error_category() };
}

std::error_code error(int ev) {
  return { ev, error_category() };
}

}  // namespace gl
