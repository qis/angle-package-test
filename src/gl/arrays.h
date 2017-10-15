#pragma once
#include <gl/error.h>
#include <gl/resource.h>
#include <memory>

namespace gl {

class arrays {
public:
  arrays() noexcept = default;

  explicit arrays(std::size_t size) : handles_(std::make_unique<GLuint[]>(size)), size_(size) {
    glGenVertexArrays(static_cast<GLsizei>(size_), handles_.get());
    if (const auto ec = error()) {
      throw system_error(ec, "Could not generate vertex array object names");
    }
  }

  arrays(arrays&& other) noexcept : size_(std::exchange(other.size_, 0)), handles_(std::move(other.handles_)) {}

  arrays& operator=(arrays&& other) noexcept {
    if (handles_) {
      glDeleteVertexArrays(static_cast<GLsizei>(size_), handles_.get());
    }
    size_ = std::exchange(other.size_, 0);
    handles_ = std::move(other.handles_);
    return *this;
  }

  ~arrays() {
    if (handles_) {
      glDeleteVertexArrays(static_cast<GLsizei>(size_), handles_.get());
    }
  }

  GLuint at(std::size_t index) const {
    if (index >= size_) {
      throw runtime_error("Array index out of range.");
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
