#include <ctime>

#include <string>
#include <vector>

#include <glad/glad.h>

#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "util.h"

struct Object {
  GLuint program;
  GLuint texture;
  GLuint vao;
  GLuint vbo;
  glm::vec3 pos{0.0f, 0.0f, 0.0f};
  glm::vec3 rot;
  glm::mat4 model;
  glm::vec2 vec;
};

void renderObject(Object &obj) {
  glBindVertexArray(obj.vao);
  glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(obj.model));
  glDrawArrays(GL_POINTS, 0, 1);
}

void recalculateObjectModelMatrix(Object &obj) {
  obj.model = glm::translate(glm::mat4(1.0f), obj.pos);
}

void moveObject(Object &obj, glm::vec3 moveVec) {
  obj.pos += moveVec;
  recalculateObjectModelMatrix(obj);
}

class App {
public:
  App(const std::string &windowTitle, int windowWidth, int windowHeight);
  ~App();

  void create(const std::string &windowTitle, int windowWidth,
              int windowHeight);
  void cleanup();
  void cleanupResources();

  void events();
  void update();
  void render();
  void run();

  void init();
  void initGLState();
  void initResources();

  void keyDown(SDL_Keycode key);

  void recalculateCamera();

private:
  SDL_Window *mWindow{nullptr};
  SDL_GLContext mContext{nullptr};
  bool mShouldClose{false};

  const Uint8 *mKeys{nullptr};

  glm::vec3 mCameraEye;
  glm::vec3 mCameraTarget;
  glm::mat4 mView;
  glm::mat4 mProjection;

  Object mDot;
};

App::App(const std::string &windowTitle, int windowWidth, int windowHeight) {
  create(windowTitle, windowWidth, windowHeight);
}

App::~App() { cleanup(); }

void App::cleanup() {
  if (!mContext)
    return;

  cleanupResources();
  SDL_GL_DeleteContext(mContext);
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}

void App::cleanupResources() {
  glDeleteBuffers(1, &mDot.vbo);
  glDeleteVertexArrays(1, &mDot.vao);
  glDeleteProgram(mDot.program);
}

void App::create(const std::string &windowTitle, int windowWidth,
                 int windowHeight) {
  cleanup();

  mWindow = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight,
                             SDL_WINDOW_OPENGL);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
  mContext = SDL_GL_CreateContext(mWindow);

  if (!gladLoadGL()) {
    printf("Failed to load GL\n");
    mShouldClose = true;
  }
}

void App::initGLState() {
  glClearColor(0.3f, 0.1f, 0.1f, 1.0f);
  glPointSize(20.0f);
  glDisable(GL_CULL_FACE);
}

void App::recalculateCamera() {
  mView = glm::lookAt(mCameraEye, mCameraEye + mCameraTarget,
                      glm::vec3(0.0f, 1.0f, 0.0f));
}

void App::init() {
  initGLState();
  initResources();

  mKeys = SDL_GetKeyboardState(nullptr);

  // mCameraEye = glm::vec3(-400.0f, -300.0f, 3);
  mCameraEye = glm::vec3(0.0f, 0.0f, 3.0f);
  mCameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
  recalculateCamera();

  mProjection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, 0.1f, 500.0f);
  // mProjection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f);
  // mProjection = glm::ortho(0.0f, 1.0f, 1.0f, 0.0f);

  GLuint dotProgram = glCreateProgram();
  GLuint dotVS = loadShader("shaders/dotVS.glsl", GL_VERTEX_SHADER);
  GLuint dotFS = loadShader("shaders/dotFS.glsl", GL_FRAGMENT_SHADER);

  glAttachShader(dotProgram, dotVS);
  glAttachShader(dotProgram, dotFS);
  linkProgram(dotProgram);
  glDeleteShader(dotVS);
  glDeleteShader(dotFS);

  printf("%d %d %d\n", glGetUniformLocation(dotProgram, "model"),
         glGetUniformLocation(dotProgram, "view"),
         glGetUniformLocation(dotProgram, "projection"));

  std::vector<GLfloat> dotVerts = {0.0f, 0.0f};

  GLuint dotVBO;
  glCreateBuffers(1, &dotVBO);

  glNamedBufferStorage(dotVBO, dotVerts.size(), dotVerts.data(),
                       GL_DYNAMIC_STORAGE_BIT);

  GLuint dotVAO;
  glCreateVertexArrays(1, &dotVAO);

  glVertexArrayVertexBuffer(dotVAO, 0, dotVBO, 0, sizeof(GLfloat) * 2);
  glEnableVertexArrayAttrib(dotVAO, 0);
  glVertexArrayAttribFormat(dotVAO, 0, 2, GL_FLOAT, GL_FALSE, 0);
  glVertexArrayAttribBinding(dotVAO, 0, 0);

  mDot.program = dotProgram;
  mDot.vbo = dotVBO;
  mDot.vao = dotVAO;
  mDot.model = glm::mat4(1.0f);
  mDot.texture = 0;
  mDot.vec = glm::vec2(static_cast<float>(((rand() % 200) - 100) / 100.0f),
                       static_cast<float>(((rand() % 200) - 100) / 100.0f));

  moveObject(mDot, glm::vec3(400.0f, 300.0f, 0.0f));
}

void App::initResources() {}

#define MOVEAMT 10.0f

void App::keyDown(SDL_Keycode key) {
  switch (key) {
  case SDLK_q:
    mShouldClose = true;
    break;
  default:
    break;
  }
}

void App::events() {
  SDL_Event ev;
  while (SDL_PollEvent(&ev)) {
    switch (ev.type) {
    case SDL_QUIT:
      mShouldClose = true;
      break;
    case SDL_KEYDOWN:
      keyDown(ev.key.keysym.sym);
      break;
    default:
      break;
    }
  }

  if (mKeys[SDL_SCANCODE_LEFT])
    moveObject(mDot, glm::vec3(-MOVEAMT, 0.0f, 0.0f));
  if (mKeys[SDL_SCANCODE_RIGHT])
    moveObject(mDot, glm::vec3(MOVEAMT, 0.0f, 0.0f));
  if (mKeys[SDL_SCANCODE_UP])
    moveObject(mDot, glm::vec3(0.0f, -MOVEAMT, 0.0f));
  if (mKeys[SDL_SCANCODE_DOWN])
    moveObject(mDot, glm::vec3(0.0f, MOVEAMT, 0.0f));
}

void App::update() {
  glm::mat4 velVec =
      glm::scale(glm::mat4(1.0f), glm::vec3(MOVEAMT, MOVEAMT, MOVEAMT));
  auto vec = velVec * glm::vec4(mDot.vec, 0, 1);
  moveObject(mDot, vec);

  if (mDot.pos.x + 10.0f >= 800.0f)
    mDot.vec.x = -mDot.vec.x;
  if (mDot.pos.x - 10.0f <= 0.0f)
    mDot.vec.x = -mDot.vec.x;
  if (mDot.pos.y + 10.0f >= 600.0f)
    mDot.vec.y = -mDot.vec.y;
  if (mDot.pos.y - 10.0f <= 0.0f)
    mDot.vec.y = -mDot.vec.y;
}

void App::render() {
  glClear(GL_COLOR_BUFFER_BIT);

  glUseProgram(mDot.program);
  glUniformMatrix4fv(2, 1, GL_FALSE, glm::value_ptr(mView));
  glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(mProjection));
  renderObject(mDot);

  SDL_GL_SwapWindow(mWindow);
}

void App::run() {
  while (!mShouldClose) {
    events();
    update();
    render();
    SDL_Delay(16);
  }
}

int main() {
  srand(time(nullptr));

  App app("DVD", 800, 600);
  app.init();
  app.run();

  return 0;
}
