#pragma once
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglplatform.h>
#include <GLES3/gl3.h>
#include <GLES2/gl2ext.h>
#include <memory>

class window {
public:
  window(int argc, char* argv[]);
  window(window&& other) = delete;
  window& operator=(window&& other) = delete;
  virtual ~window();

  int run() noexcept;
  void show(bool show) noexcept;

  EGLNativeWindowType native_window() const;
  EGLNativeDisplayType nateive_display() const;

  virtual void on_create(GLsizei cx, GLsizei cy, GLint dpi) = 0;
  virtual void on_resize(GLsizei cx, GLsizei cy, GLint dpi) = 0;
  virtual void on_destroy() = 0;
  virtual void on_render() = 0;

private:
  class impl;
  std::unique_ptr<impl> impl_;
};
