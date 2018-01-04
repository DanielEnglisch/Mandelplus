#pragma once
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>


static  double map(double value, double inMin, double inMax, double outMin, double outMax) {
	return outMin + (outMax - outMin) * (value - inMin) / (inMax - inMin);
}

static unsigned int CompileShader(unsigned int type, const std::string& source) {
	unsigned int id = glCreateShader(type);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	/* Error checking */
	int res;
	glGetShaderiv(id, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		int len;
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);
		char* message = (char*)alloca(len * sizeof(char));
		glGetShaderInfoLog(id, len, &len, message);
		std::cout << "Error compiling " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader: " << message << std::endl;
		glDeleteShader(id);
		return 0;
	}

	return id;
}

static void setUniform(const unsigned int programId, const std::string& uniformName, const double value) {
	unsigned int loc = glGetUniformLocation(programId, uniformName.c_str());
	glUniform1d(loc, value);
}

static unsigned int CreateProgram(const std::string& vert, const std::string& frag) {
	unsigned int programId = glCreateProgram();
	unsigned int vShader = CompileShader(GL_VERTEX_SHADER, vert);
	unsigned int fShader = CompileShader(GL_FRAGMENT_SHADER, frag);
	glAttachShader(programId, vShader);
	glAttachShader(programId, fShader);
	glLinkProgram(programId);
	glValidateProgram(programId);
	glDetachShader(programId, vShader);
	glDetachShader(programId, fShader);

	return programId;
}
