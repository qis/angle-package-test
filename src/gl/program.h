#pragma once
#include <gl/error.h>
#include <gl/resource.h>
#include <gl/shader.h>
#include <functional>
#include <string>
#include <string_view>

namespace gl {

class program {
public:
  program() noexcept = default;

  explicit program(const shader& vert, const shader& frag) {
    handle_.reset(glCreateProgram());
    if (const auto ec = error()) {
      throw system_error(ec, "Could not create program");
    }

    glAttachShader(handle_, vert);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not attach vertex shader");
    }

    glAttachShader(handle_, frag);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not attach fragment shader");
    }

    glLinkProgram(handle_);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not link program");
    }

    glDetachShader(handle_, frag);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not detach fragment shader");
    }

    glDetachShader(handle_, vert);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not detach vertex shader");
    }

    GLint success = GL_FALSE;
    glGetProgramiv(handle_, GL_LINK_STATUS, &success);
    if (const auto ec = error()) {
      throw system_error(ec, "Could not get program link status");
    }

    if (!success) {
      std::string info;
      GLsizei size = 0;
      glGetProgramiv(handle_, GL_INFO_LOG_LENGTH, &size);
      if (!error()) {
        info.resize(size);
        glGetProgramInfoLog(handle_, size, &size, &info[0]);
        if (!error()) {
          info.resize(size);
        } else {
          info.clear();
        }
      }
      throw runtime_error("Shader compilation failed.\n" + info + "\n\n" + vert.source() + "\n\n" + frag.source());
    }
  }

  explicit program(std::string_view vert, std::string_view frag) :
    program(gl::shader(vert, GL_VERTEX_SHADER), gl::shader(frag, GL_FRAGMENT_SHADER)) {}

  GLint attribute(const char* name) const noexcept {
    return glGetAttribLocation(handle_, name);
  }

  GLint uniform(const char* name) const noexcept {
    return glGetUniformLocation(handle_, name);
  }

  operator GLuint() const noexcept {
    return handle_;
  }

  static void release(GLuint handle) noexcept {
    glDeleteProgram(handle);
  }

private:
  resource<GLuint, program> handle_;
};

}  // namespace gl
