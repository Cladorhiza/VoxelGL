#include "ComputeShader.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "FileUtil.h"
#include "Shader.h"

ComputeShader::ComputeShader(std::string shaderPath)
{
	const std::string computeSource = FileUtil::ParseFileToString(shaderPath);
	rendererID = CreateShader(computeSource);
}

void ComputeShader::Bind() const {
	glUseProgram(rendererID);
}

void ComputeShader::Unbind() const {
	glUseProgram(rendererID);
}

unsigned int ComputeShader::CompileShader(const std::string& source) {

	unsigned int id = glCreateShader(GL_COMPUTE_SHADER);
	const char* src = source.c_str();
	glShaderSource(id, 1, &src, nullptr);
	glCompileShader(id);

	int result;
	glGetShaderiv(id, GL_COMPILE_STATUS, &result);
	if (result == GL_FALSE) {
		int length = 0;
		char message[4096];
		glGetShaderInfoLog(id, 4096, &length, message);
		
		std::cout << "Failed to compile compute shader: " << message << '\n';
		glDeleteShader(id);
		return 0;
	}

	return id;
}

unsigned int ComputeShader::CreateShader(const std::string& computeSource) {

	const unsigned int program = glCreateProgram();
	const unsigned int computeShader = CompileShader(computeSource);

	glAttachShader(program, computeShader);
	glLinkProgram(program);
	glValidateProgram(program);
	glDeleteShader(computeShader);

	return program;
}