#include "util.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <streambuf>
#include <vector>

std::string loadTextFile(const std::string &path)
{
    std::ifstream file(path);
    if (!file.good())
        throw std::runtime_error("Failed to open file: " + path);
    return std::string((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
}

GLuint loadShader(const std::string &path, GLenum type)
{
    auto src = loadTextFile(path);
    const char *src_c = src.c_str();

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src_c, NULL);
    glCompileShader(shader);

    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE) {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

        std::string error(errorLog.begin(), errorLog.end());
        std::cerr << path + ":\n" + error << std::endl;

        glDeleteShader(shader);

        throw std::runtime_error("Problem with shader.");
    }

    return shader;
}

void linkProgram(GLuint program)
{
    glLinkProgram(program);
    GLint isLinked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<GLchar> errorLog(maxLength);
        glGetProgramInfoLog(program, maxLength, &maxLength, &errorLog[0]);

        std::string error(errorLog.begin(), errorLog.end());
        std::cerr << "Info: " + error << std::endl;

        glDeleteProgram(program);

        throw std::runtime_error("Problem with shader program.");
    }
}

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

struct StbImage {
    StbImage(const std::string &path)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        stbi_set_flip_vertically_on_load(false);
        data = stbi_load(path.c_str(), &width, &height, &numComponents, 0);
        if (!data) {
            std::stringstream error;
            error << "Image (" << path << ") failed to load:\n";
            error << stbi_failure_reason() << std::endl;
            throw std::runtime_error(error.str());
        }
    }
    ~StbImage()
    {
        stbi_image_free(data);
    }

    int width {0};
    int height {0};
    int numComponents {0};
    stbi_uc *data {nullptr};
};

GLenum componentsToFormat(int numComponents)
{
    switch (numComponents) {
        case 1:
            return GL_RED;
        case 3:
            return GL_RGB;
        case 4:
            return GL_RGBA;
        default:
            return GL_RGBA;
    }
    return GL_RGBA;
}

GLuint loadTexture(const std::string &filePath, int &width, int &height)
{
    StbImage image(filePath);

    GLenum format = componentsToFormat(image.numComponents);

    GLuint handle {GL_NONE};
    glCreateTextures(GL_TEXTURE_2D, 1, &handle);

    glTextureStorage2D(handle, 1, GL_RGBA8, image.width, image.height);
    glTextureSubImage2D(handle, 0, 0, 0, image.width, image.height, format, GL_UNSIGNED_BYTE, image.data);

    glTextureParameteri(handle, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTextureParameteri(handle, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTextureParameteri(handle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTextureParameteri(handle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    width = image.width;
    height = image.height;

    return handle;
}
