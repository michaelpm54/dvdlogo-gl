#ifndef UTIL_H
#define UTIL_H

#include <string>

#include <glad/glad.h>

std::string loadTextFile(const std::string &path);
GLuint loadShader(const std::string &path, GLenum type);
void linkProgram(GLuint program);

#endif // UTIL_H
