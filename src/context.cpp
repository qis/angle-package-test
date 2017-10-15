#include "context.h"
#include <egl/egl.h>
#include <egl/eglext.h>
#include <egl/eglplatform.h>
#include <egl/error.h>
#include <chrono>

void context::on_create(GLsizei cx, GLsizei cy, GLint dpi) {
  // Create OpenGL ES display.
  // TODO: Set EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE to EGL_EXPERIMENTAL_PRESENT_PATH_FAST_ANGLE and
  //       fall back to EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE if eglChooseConfig fails.
  const EGLint platform_attributes[] = {
    EGL_PLATFORM_ANGLE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_TYPE_D3D11_ANGLE,
    EGL_PLATFORM_ANGLE_DEVICE_TYPE_ANGLE, EGL_PLATFORM_ANGLE_DEVICE_TYPE_HARDWARE_ANGLE,
    EGL_EXPERIMENTAL_PRESENT_PATH_ANGLE, EGL_EXPERIMENTAL_PRESENT_PATH_COPY_ANGLE,
    EGL_PLATFORM_ANGLE_ENABLE_AUTOMATIC_TRIM_ANGLE, EGL_TRUE,
    EGL_NONE,
  };
  display_ = eglGetPlatformDisplayEXT(EGL_PLATFORM_ANGLE_ANGLE, nateive_display(), platform_attributes);
  if (display_ == EGL_NO_DISPLAY) {
    display_ = eglGetDisplay(nateive_display());
  }
  if (display_ == EGL_NO_DISPLAY) {
    throw egl::system_error(egl::error(), "Could not create OpenGL ES display");
  }
  if (!eglInitialize(display_, nullptr, nullptr)) {
    throw egl::system_error(egl::error(), "Could not initialize OpenGL ES display");
  }

  // Choose OpenGL ES configuration.
  EGLConfig config = {};
  EGLint config_count = 0;
  EGLint attributes[] = {
    EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
    EGL_CONFORMANT, EGL_OPENGL_ES3_BIT,
    EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
    EGL_RED_SIZE, 8,
    EGL_GREEN_SIZE, 8,
    EGL_BLUE_SIZE, 8,
    EGL_ALPHA_SIZE, 0,
    EGL_STENCIL_SIZE, 8,
    EGL_NONE
  };
  if (!eglChooseConfig(display_, attributes, &config, 1, &config_count)) {
    throw egl::system_error(egl::error(), "Could not choose OpenGL ES config");
  }
  if (config_count < 1) {
    throw egl::runtime_error("Could not chose a valid OpenGL ES 3 config.");
  }

  // Bind OpenGL ES API.
  if (!eglBindAPI(EGL_OPENGL_ES_API)) {
    throw egl::system_error(egl::error(), "Could not bind OpenGL ES API");
  }

  // Create OpenGL ES surface.
  surface_ = eglCreateWindowSurface(display_, config, native_window(), nullptr);
  if (surface_ == EGL_NO_SURFACE) {
    throw egl::system_error(egl::error(), "Could not create OpenGL ES surface");
  }

  // Create OpenGL ES context.
  const EGLint ctxattr[] = {
    EGL_CONTEXT_CLIENT_VERSION, 3,
    EGL_NONE
  };
  context_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, ctxattr);
  if (context_ == EGL_NO_CONTEXT) {
    throw egl::system_error(egl::error(), "Could not create OpenGL ES context");
  }
  if (!eglMakeCurrent(display_, surface_, surface_, context_)) {
    throw egl::system_error(egl::error(), "Could not attach OpenGL ES context");
  }

  // Create renderbuffer and framebuffer for multisampling.
  if (samples_ > 1) {
    // Create renderbuffer.
    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples_, GL_BGRA8_EXT, cx, cy);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Create framebuffer and set renderbuffer.
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  // Create scene.
  create(cx, cy, dpi);
  resize(cx, cy, dpi);

  // Show window.
  show(true);
}

void context::on_resize(GLsizei cx, GLsizei cy, GLint dpi) {
  cx_ = cx;
  cy_ = cy;
  if (samples_ > 1) {
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples_, GL_BGRA8_EXT, cx, cy);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glGetError();
  }
  resize(cx, cy, dpi);
  static auto beg = std::chrono::system_clock::now();
  auto now = std::chrono::system_clock::now();
  if (now - beg > std::chrono::milliseconds(16)) {
    on_render();
    beg = now;
  }
}

void context::on_destroy() {
  // Destroy framebuffer and renderbuffer.
  if (samples_ > 1) {
    glDeleteFramebuffers(1, &fbo_);
    glDeleteRenderbuffers(1, &rbo_);
  }

  // Destroy OpenGL ES display, context and surface.
  if (display_ != EGL_NO_DISPLAY) {
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(display_, context_);
      context_ = EGL_NO_CONTEXT;
    }
    if (surface_ != EGL_NO_SURFACE) {
      eglDestroySurface(display_, surface_);
      surface_ = EGL_NO_SURFACE;
    }
    eglTerminate(display_);
    display_ = EGL_NO_DISPLAY;
  }
}

void context::on_render() {
  // Set framebuffer when multisampling is enabled.
  if (samples_ > 1) {
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  }

  // Render scene.
  render();

  // Unset framebuffer when multisampling is enabled.
  if (samples_ > 1) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebuffer(0, 0, cx_, cy_, 0, 0, cx_, cy_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  }

  // Swap buffers.
  eglSwapBuffers(display_, surface_);
}
