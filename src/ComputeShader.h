#pragma once
#include <string>
#include <GL/glew.h>

class ComputeShader
{

public:

	ComputeShader(std::string shaderPath);

	void Bind() const;
	void Unbind() const;

	static unsigned int CreateShader(const std::string& computeSource);
	static unsigned int CompileShader(const std::string& source);
private:

	unsigned int rendererID;
};
