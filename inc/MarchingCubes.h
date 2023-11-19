#pragma once
#include <vector>
#include "vec3.hpp"

namespace MarchingCubes
{
	std::vector<glm::vec3> MarchCubes(const std::vector<std::vector<std::vector<float>>>& gridValues, 
									  float surfaceValue, 
									  std::vector<glm::vec3>& outNormals);
};

