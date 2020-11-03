#ifndef UTIL_H
#define UTIL_H

#include "app_gl.h"

#include <string>

std::string loadTextFile(const std::string &path);
GLuint loadShader(const std::string &path, GLenum type);
void linkProgram(GLuint program);
GLuint loadTexture(const std::string &filePath, int &width, int &height);

#endif    // UTIL_H
