#pragma once
#include <GLES3/gl3.h>
#include <utility>

namespace gl {

template <typename T, typename I>
class resource {
public:
  resource() noexcept = default;
  resource(T handle) noexcept : handle_(handle) {}

  resource(resource&& other) noexcept : handle_(std::exchange(other.handle_, 0)) {}

  resource& operator=(resource&& other) noexcept {
    if (handle_) {
      I::release(handle_);
    }
    handle_ = std::exchange(other.handle_, 0);
    return *this;
  }

  ~resource() {
    if (handle_) {
      I::release(handle_);
    }
  }

  void reset(GLuint handle) noexcept {
    if (handle_) {
      I::release(handle_);
    }
    handle_ = handle;
  }

  GLuint release() noexcept {
    return std::exchange(handle_, 0);
  }

  operator T() const noexcept {
    return handle_;
  }

  friend bool operator==(const resource& lhs, const resource& rhs) noexcept {
    return lhs.handle_ == rhs.handle_;
  }

  friend bool operator!=(const resource& lhs, const resource& rhs) noexcept {
    return lhs.handle_ != rhs.handle_;
  }

  friend bool operator<(const resource& lhs, const resource& rhs) noexcept {
    return lhs.handle_ < rhs.handle_;
  }

  friend bool operator>(const resource& lhs, const resource& rhs) noexcept {
    return lhs.handle_ > rhs.handle_;
  }

  friend bool operator<=(const resource& lhs, const resource& rhs) noexcept {
    return lhs.handle_ <= rhs.handle_;
  }

  friend bool operator>=(const resource& lhs, const resource& rhs) noexcept {
    return lhs.handle_ >= rhs.handle_;
  }

private:
  T handle_ = 0;
};

}  // namespace gl
