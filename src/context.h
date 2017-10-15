#pragma once
#include <window.h>
#include <GLES3/gl3.h>

class context : public window {
public:
  using window::window;

  virtual void create(GLsizei cx, GLsizei cy, GLint dpi) = 0;
  virtual void resize(GLsizei cx, GLsizei cy, GLint dpi) = 0;
  virtual void destroy() = 0;
  virtual void render() = 0;

  void on_create(GLsizei cx, GLsizei cy, GLint dpi) override;
  void on_resize(GLsizei cx, GLsizei cy, GLint dpi) override;
  void on_destroy() override;
  void on_render() override;

private:
  EGLDisplay display_ = EGL_NO_DISPLAY;
  EGLSurface surface_ = EGL_NO_SURFACE;
  EGLContext context_ = EGL_NO_CONTEXT;

  GLsizei samples_ = 1;
  GLuint rbo_ = 0;
  GLuint fbo_ = 0;

  GLsizei cx_ = 1;
  GLsizei cy_ = 1;
  GLint dpi_ = 96;

  bool resize_ = true;
};
