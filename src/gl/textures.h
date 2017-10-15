#pragma once
#include <gl/error.h>
#include <GLES3/gl3.h>
#include <memory>

namespace gl {

class textures {
public:
  textures() noexcept = default;

  explicit textures(std::size_t size) : handles_(std::make_unique<GLuint[]>(size)), size_(size) {
    glGenTextures(static_cast<GLsizei>(size_), handles_.get());
    if (const auto ec = error()) {
      throw system_error(ec, "Could not generate texture names");
    }
  }

  textures(textures&& other) noexcept : size_(std::exchange(other.size_, 0)), handles_(std::move(other.handles_)) {}

  textures& operator=(textures&& other) noexcept {
    if (handles_) {
      glDeleteTextures(static_cast<GLsizei>(size_), handles_.get());
    }
    size_ = std::exchange(other.size_, 0);
    handles_ = std::move(other.handles_);
    return *this;
  }

  ~textures() {
    if (handles_) {
      glDeleteTextures(static_cast<GLsizei>(size_), handles_.get());
    }
  }

  GLuint at(std::size_t index) const {
    if (index >= size_) {
      throw runtime_error("Texture index out of range.");
    }
    return handles_[index];
  }

  GLuint operator[](std::size_t index) const noexcept {
    return handles_[index];
  }

private:
  std::unique_ptr<GLuint[]> handles_;
  std::size_t size_ = 0;
};

}  // namespace gl
