#pragma once

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
namespace Camera{

	
	void Update();
	glm::mat4 GetViewMatrix();
	float GetZoomSensitivity();
};

