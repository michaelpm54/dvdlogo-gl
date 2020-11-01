#include "util.h"

#include <stdexcept>
#include <fstream>
#include <vector>
#include <iostream>
#include <streambuf>

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
	if(isCompiled == GL_FALSE)
	{
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
	if(isLinked == GL_FALSE)
	{
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
