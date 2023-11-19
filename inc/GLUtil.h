#pragma once
#include <vec2.hpp>
#include <vec3.hpp>
#include <vector>

namespace GLUtil {

    unsigned BuildVAOfromData(const std::vector<float>& vertexes, 
                                const std::vector<float>& colours, 
                                const std::vector<unsigned>& indexes);

	unsigned BuildVAOfromData(const std::vector<glm::vec3>& vertexes, 
                                const std::vector<glm::vec3>& colours, 
                                const std::vector<unsigned>& indexes);

    unsigned BuildVAOfromData(const std::vector<glm::vec3>& vertexes, 
                                const std::vector<glm::vec3>& colours, 
                                const std::vector<unsigned>& indexes,
								const std::vector<glm::vec3>& normals);

	unsigned BuildVAOfromData(const std::vector<glm::vec3>& vertexes, 
								const std::vector<glm::vec2>& texCoords);
};