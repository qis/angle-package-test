#include "context.h"
#include <egl/egl.h>
#include <egl/eglext.h>
#include <egl/eglplatform.h>
#include <egl/error.h>
#include <gl/error.h>

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

  // Create render buffer and frame buffer for multisampling.
  if (samples_ > 1) {
    glGenRenderbuffers(1, &rbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
    glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, samples_, GL_BGRA8_EXT, cx, cy);
    glGenFramebuffers(1, &fbo_);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo_);
    if (glGetError()) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      glDeleteFramebuffers(1, &fbo_);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
      glDeleteRenderbuffers(1, &rbo_);
      glGetError();
      samples_ = 0;
    }
  }

  // Initialize dpi and size.
  on_resize(cx, cy, dpi);

  // Create scene.
  create(cx, cy, dpi);

  // Show window.
  show(true);
}

void context::on_resize(GLsizei cx, GLsizei cy, GLint dpi) {
  cx_ = cx;
  cy_ = cy;
  dpi_ = dpi;
  resize_ = true;
}

void context::on_destroy() {
  // Destroy render buffer and frame buffer.
  if (samples_ > 1) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &fbo_);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
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
  // Resize client.
  if (resize_) {
    resize(cx_, cy_, dpi_);
    if (samples_ > 1) {
      glRenderbufferStorageMultisampleANGLE(GL_RENDERBUFFER, samples_, GL_BGRA8_EXT, cx_, cy_);
    }
    resize_ = false;
  }

  // Set framebuffer when multisampling is enabled.
  if (samples_ > 1) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo_);
  }

  // Render scene.
  if (const auto ec = gl::error()) {
    throw gl::system_error(ec, "Could not setup frame");
  }
  render();

  // Unset framebuffer when multisampling is enabled.
  if (samples_ > 1) {
    glBindFramebuffer(GL_READ_FRAMEBUFFER, fbo_);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    glBlitFramebufferANGLE(0, 0, cx_, cy_, 0, 0, cx_, cy_, GL_COLOR_BUFFER_BIT, GL_NEAREST);
  }

  // Swap buffers.
  eglSwapBuffers(display_, surface_);
  if (const auto ec = gl::error()) {
    throw gl::system_error(ec, "Could not finish frame");
  }
}
