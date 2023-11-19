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

	void SetUniform1f(const std::string& name, float f);
private:

	unsigned int rendererID;
};
