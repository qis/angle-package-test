#pragma once
#include <stdexcept>
#include <system_error>

namespace gl {

const std::error_category& error_category();

std::error_code error();
std::error_code error(int ev);

class runtime_error : public std::runtime_error {
public:
  using std::runtime_error::runtime_error;
};

class system_error : public std::system_error {
public:
  std::system_error::system_error;
};

}  // namespace gl
