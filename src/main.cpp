#include <glad/glad.h>

#include <ctime>
#include <string>
#include <vector>

#define SDL_MAIN_HANDLED
#include <SDL.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "util.h"

static constexpr int kWindowWidth {800};
static constexpr int kWindowHeight {600};
static constexpr float kMoveSpeed {4.0f};

struct Object {
    GLuint texture;
    GLuint vao;
    GLuint vbo;
    glm::vec3 pos {0.0f, 0.0f, 0.0f};
    glm::vec3 rot;
    glm::mat4 model {1.0f};
    glm::vec2 velocity;
    int width;
    int height;

    void render(GLuint program)
    {
        glBindVertexArray(vao);
        glUniform1i(glGetUniformLocation(program, "tex"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glDrawArrays(GL_POINTS, 0, 1);
    }

    void updateTransform()
    {
        model = glm::translate(glm::mat4(1.0f), pos);
    }

    void moveDir(glm::vec3 vec)
    {
        pos += vec * kMoveSpeed;
        updateTransform();
    }

    void setPosition(glm::vec3 vec)
    {
        pos = vec;
        updateTransform();
    }

    void setVector(glm::vec3 vec)
    {
        velocity = vec;
    }
};

class App {
public:
    App(const std::string &windowTitle, int windowWidth, int windowHeight);
    ~App();

    void create(const std::string &windowTitle, int windowWidth, int windowHeight);
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
    SDL_Window *mWindow {nullptr};
    SDL_GLContext mContext {nullptr};
    bool mShouldClose {false};

    const Uint8 *mKeys {nullptr};

    glm::vec3 mCameraEye;
    glm::vec3 mCameraTarget;
    glm::mat4 mView;
    glm::mat4 mProjection;

    Object mLogo;

    GLuint mProgram;
};

App::App(const std::string &windowTitle, int windowWidth, int windowHeight)
{
    create(windowTitle, windowWidth, windowHeight);
}

App::~App()
{
    cleanup();
}

void App::cleanup()
{
    if (!mContext)
        return;

    cleanupResources();
    SDL_GL_DeleteContext(mContext);
    SDL_DestroyWindow(mWindow);
    SDL_Quit();
}

void App::cleanupResources()
{
    glDeleteProgram(mProgram);
    glDeleteBuffers(1, &mLogo.vbo);
    glDeleteVertexArrays(1, &mLogo.vao);
    glDeleteTextures(1, &mLogo.texture);
}

void App::create(const std::string &windowTitle, int windowWidth, int windowHeight)
{
    cleanup();

    mWindow = SDL_CreateWindow(windowTitle.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, SDL_WINDOW_OPENGL);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    mContext = SDL_GL_CreateContext(mWindow);

    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGL()) {
        printf("Failed to load GL\n");
        mShouldClose = true;
    }
}

void App::initGLState()
{
    glClearColor(0.3f, 0.1f, 0.1f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void App::recalculateCamera()
{
    mView = glm::lookAt(mCameraEye, mCameraEye + mCameraTarget, glm::vec3(0.0f, 1.0f, 0.0f));
}

void App::init()
{
    initGLState();
    initResources();

    mKeys = SDL_GetKeyboardState(nullptr);

    /* VIEW */
    mCameraEye = glm::vec3(0.0f, 0.0f, 3.0f);
    mCameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
    recalculateCamera();

    mProjection = glm::ortho(0.0f, static_cast<float>(kWindowWidth), static_cast<float>(kWindowHeight), 0.0f, 0.1f, 500.0f);

    /* SHADERS */
    mProgram = glCreateProgram();

    GLuint vertexShader = loadShader("shaders/dotVS.glsl", GL_VERTEX_SHADER);
    GLuint fragmentShader = loadShader("shaders/dotFS.glsl", GL_FRAGMENT_SHADER);

    glAttachShader(mProgram, vertexShader);
    glAttachShader(mProgram, fragmentShader);
    linkProgram(mProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    /* OBJECT & GEOMETRY */
    std::vector<GLfloat> vertices = {0.0f, 0.0f};
    glCreateBuffers(1, &mLogo.vbo);
    glNamedBufferStorage(mLogo.vbo, vertices.size(), vertices.data(), 0);
    glCreateVertexArrays(1, &mLogo.vao);
    glVertexArrayVertexBuffer(mLogo.vao, 0, mLogo.vbo, 0, sizeof(GLfloat) * 2);
    glEnableVertexArrayAttrib(mLogo.vao, 0);
    glVertexArrayAttribFormat(mLogo.vao, 0, 2, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(mLogo.vao, 0, 0);
    mLogo.model = glm::mat4(1.0f);

    float initialXVec = 0.2f + static_cast<float>(rand() % 10) / 8.0f;
    float initialYVec = 0.2f + static_cast<float>(rand() % 10) / 8.0f;

    mLogo.setVector(glm::normalize(glm::vec3 {initialXVec, initialYVec, 0.0f}));
    mLogo.setPosition({400.0f, 300.0f, 0.0f});
}

void App::initResources()
{
    mLogo.texture = loadTexture("logo.png", mLogo.width, mLogo.height);
    glPointSize(static_cast<float>(std::max(mLogo.width, mLogo.height)));
}

void App::keyDown(SDL_Keycode key)
{
    switch (key) {
        case SDLK_q:
            mShouldClose = true;
            break;
        default:
            break;
    }
}

void App::events()
{
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
        mLogo.moveDir({-1.0f, 0.0f, 0.0f});
    if (mKeys[SDL_SCANCODE_RIGHT])
        mLogo.moveDir({1.0f, 0.0f, 0.0f});
    if (mKeys[SDL_SCANCODE_UP])
        mLogo.moveDir({0.0f, -1.0f, 0.0f});
    if (mKeys[SDL_SCANCODE_DOWN])
        mLogo.moveDir({0.0f, 1.0f, 0.0f});
}

void App::update()
{
    auto vec = glm::vec4(mLogo.velocity, 0, 1);

    mLogo.moveDir(vec);

    if (mLogo.pos.x + (mLogo.width - 60) >= kWindowWidth)
        vec.x = -vec.x;
    if (mLogo.pos.x - (mLogo.width - 60) <= 0.0)
        vec.x = -vec.x;
    if (mLogo.pos.y + (mLogo.height / 2) + 5 >= kWindowHeight)
        vec.y = -vec.y;
    if (mLogo.pos.y - (mLogo.height / 2) <= 0.0)
        vec.y = -vec.y;

    mLogo.setVector(vec);
}

void App::render()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(mProgram);
    glUniformMatrix4fv(glGetUniformLocation(mProgram, "view"), 1, GL_FALSE, glm::value_ptr(mView));
    glUniformMatrix4fv(glGetUniformLocation(mProgram, "projection"), 1, GL_FALSE, glm::value_ptr(mProjection));
    mLogo.render(mProgram);

    SDL_GL_SwapWindow(mWindow);
}

void App::run()
{
    while (!mShouldClose) {
        events();
        update();
        render();
    }
}

int main()
{
    srand(static_cast<unsigned int>(time(nullptr)));

    App app("DVD", kWindowWidth, kWindowHeight);
    app.init();
    app.run();

    return 0;
}
