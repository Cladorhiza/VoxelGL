#include "MarchingCubes.h"

#include <vector>
#include <array>
#include <iostream>

#include "vec3.hpp"
#include "glm.hpp"

void MarchingCubes::MarchCube(std::vector<glm::vec3>& outVertexes, std::array<float,8>& cubeData, float surfaceValue, std::vector<glm::vec3>& outNormals) {

	int16_t vertexesBelowSurface{0};

	for (int16_t i{0}; i< 8; ++i) {
		if (cubeData[i] < surfaceValue) {
			vertexesBelowSurface |= 1 << i;
		}
	}

	const int* tris = triTable[vertexesBelowSurface];

	for (int16_t i{0}; i+2 < 16; i+=3) {
		//if -1, end of vertexes
		if (tris[i] == -1){
			break;
		}

		outVertexes.emplace_back(edgeVertexes[tris[i]][0],edgeVertexes[tris[i]][1] ,edgeVertexes[tris[i]][2]);
		outVertexes.emplace_back(edgeVertexes[tris[i+1]][0],edgeVertexes[tris[i+1]][1] ,edgeVertexes[tris[i+1]][2]);
		outVertexes.emplace_back(edgeVertexes[tris[i+2]][0],edgeVertexes[tris[i+2]][1] ,edgeVertexes[tris[i+2]][2]);
		glm::vec3 norm1{outVertexes[outVertexes.size()-2] - outVertexes[outVertexes.size()-3]};
		glm::vec3 norm2{outVertexes[outVertexes.size()-1] - outVertexes[outVertexes.size()-2]};
		glm::vec3 normal{glm::cross(norm1, norm2)};
		normal = glm::normalize(normal);
		//std::cout << "X: " << normal.x << " Y: " << normal.y << " Z: " << normal.z << '\n';
		outNormals.emplace_back(normal);
		outNormals.emplace_back(normal);
		outNormals.emplace_back(normal);
	}
}
//TODO: Remove duplicate vertexes, map? or cache border indexes while looping over them
std::vector<glm::vec3> MarchingCubes::MarchCubes(const std::vector<std::vector<std::vector<float>>>& gridValues, float surfaceValue, std::vector<glm::vec3>& outNormals) {

	std::vector<glm::vec3> vertexes;

	for (size_t i{0}; i < gridValues.size()-1; ++i) {
	    for (size_t j{0}; j < gridValues[i].size()-1; ++j) {
            for (size_t k{0}; k < gridValues[i][j].size()-1; ++k) {
				std::array<float,8> cubeVals {
					gridValues[i][j][k],
					gridValues[i+1][j][k],
					gridValues[i+1][j][k+1],
					gridValues[i][j][k+1],
					gridValues[i][j+1][k],
					gridValues[i+1][j+1][k],
					gridValues[i+1][j+1][k+1],
					gridValues[i][j+1][k+1]
				};

				size_t oldVecSize{vertexes.size()};

				MarchCube(vertexes, cubeVals, surfaceValue, outNormals);

				size_t vecSizeDifference {vertexes.size() - oldVecSize};

				for (size_t l{vertexes.size() - vecSizeDifference}; l < vertexes.size(); l++) {

					//add offset of grid values to vertex positions
					glm::vec3 delta{i,j,k};
					vertexes[l] += delta;
				}
            }
	    }
    }
	return vertexes;
}