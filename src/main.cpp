#include <context.h>
#include <gl/error.h>
#include <gl/arrays.h>
#include <gl/buffers.h>
#include <gl/program.h>
#include <string_view>

class client : public context {
public:
  using context::context;

  void create(GLsizei cx, GLsizei cy, GLint dpi) override {
    // Create scene.
    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.2f, 0.4f, 0.6f, 1.0f);

    // Load resources.
    const std::string_view vert =
      "#version 300 es\n"
      "precision mediump float;\n"
      "in vec2 position;\n"
      "in vec3 color;\n"
      "out vec3 vert_color;\n"
      "void main() {\n"
      "  vert_color = color;\n"
      "  gl_Position = vec4(position, 0.0, 1.0);\n"
      "}";

    const std::string_view frag =
      "#version 300 es\n"
      "precision mediump float;\n"
      "in vec3 vert_color;\n"
      "out vec4 frag_color;\n"
      "void main() {\n"
      "  frag_color = vec4(vert_color, 1.0);\n"
      "}";

    // Create scene objects.
    vao_ = gl::arrays(1);
    vbo_ = gl::buffers(2);
    program_ = gl::program(vert, frag);

    // Get program attribute and uniform locations.
    glUseProgram(program_);
    const auto position = program_.attribute("position");
    const auto color = program_.attribute("color");

    // Create vertices VBO.
    const float vertices[] = {
    // Position     Color
       0.0f,  0.5f, 1.0f, 0.0f, 0.0f,
       0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
      -0.5f, -0.5f, 0.0f, 0.0f, 1.0f
    };
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Create elements VBO.
    const unsigned elements[] = {
      0, 1, 2
    };
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);

    // Add enabled attributes (including their layout information and currently boud buffers) to the VAO.
    glBindVertexArray(vao_[0]);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_[0]);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_[1]);
    glEnableVertexAttribArray(position);
    glVertexAttribPointer(position, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glEnableVertexAttribArray(color);
    glVertexAttribPointer(color, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
  }

  void resize(GLsizei cx, GLsizei cy, GLint dpi) override {
    glViewport(0, 0, cx, cy);
  }

  void destroy() override {
  }

  void render() override {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(program_);
    glBindVertexArray(vao_[0]);
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
    //if (const auto ec = gl::error()) {
    //  throw gl::system_error(ec, "Could not render frame");
    //}
  }

private:
  gl::arrays vao_;
  gl::buffers vbo_;
  gl::program program_;
};

int main(int argc, char* argv[]) {
  client client(argc, argv);
  return client.run();
}

#ifdef WIN32
#include <windows.h>
#include <shellapi.h>
#include <string>
#include <vector>

int wWinMain(HINSTANCE instance, HINSTANCE, LPWSTR cmd, int show) {
  auto argc = 0;
  auto argw = CommandLineToArgvW(cmd, &argc);
  std::vector<char*> argv;
  std::vector<std::string> data;
  for (auto i = 0; i < argc; ++i) {
    std::string arg;
    arg.resize(WideCharToMultiByte(CP_UTF8, 0, argw[i], -1, nullptr, 0, nullptr, nullptr) + 1);
    arg.resize(WideCharToMultiByte(CP_UTF8, 0, argw[i], -1, arg.data(), static_cast<int>(arg.size()), nullptr, nullptr));
    argv.push_back(data.insert(data.end(), std::move(arg))->data());
  }
  return main(argc, argv.data());
}
#endif
