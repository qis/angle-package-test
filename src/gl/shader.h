#pragma once
#include <gl/error.h>
#include <gl/resource.h>
#include <string>
#include <string_view>

namespace gl {

class shader {
public:
  shader() noexcept = default;

  explicit shader(std::string_view src, GLenum type) {
    handle_.reset(glCreateShader(type));
    if (const auto ec = error()) {
      throw system_error(ec, "Could not create shader object");
    }

    auto data = src.data();
    auto size = static_cast<GLint>(src.size());
    glShaderSource(handle_, 1, &data, &size);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not set shader source");
    }

    glCompileShader(handle_);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not compile the shader.");
    }

    GLint success = GL_FALSE;
    glGetShaderiv(handle_, GL_COMPILE_STATUS, &success);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not get shader compile status.");
    }

    if (!success) {
      std::string info;
      GLsizei size = 0;
      glGetShaderiv(handle_, GL_INFO_LOG_LENGTH, &size);
      if (!error()) {
        info.resize(size);
        glGetShaderInfoLog(handle_, size, &size, &info[0]);
        if (!error()) {
          info.resize(size);
        } else {
          info.clear();
        }
      }
      throw runtime_error("Shader compilation failed.\n" + info + "\n" + std::string(src));
    }
  }

  std::string source() const {
    if (!handle_ || !glIsShader(handle_)) {
      return{};
    }
    GLsizei size = 0;
    glGetShaderiv(handle_, GL_SHADER_SOURCE_LENGTH, &size);
    if (size) {
      return{};
    }
    std::string src;
    src.resize(size);
    glGetShaderSource(handle_, size, &size, &src[0]);
    src.resize(size);
    return src;
  }

  operator GLuint() const noexcept {
    return handle_;
  }

  static void release(GLuint handle) noexcept {
    glDeleteShader(handle);
  }

private:
  resource<GLuint, shader> handle_;
};

}  // namespace gl
