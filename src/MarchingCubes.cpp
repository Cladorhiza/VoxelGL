#include "MarchingCubes.h"

#include <vector>
#include <array>
#include "vec3.hpp"

void MarchingCubes::MarchCube(std::vector<glm::vec3>& outVertexes, std::array<float,8>& cubeData, float surfaceValue) {

	int16_t vertexesBelowSurface{0};

	for (int16_t i{0}; i< 8; ++i) {
		if (cubeData[i] < surfaceValue) {
			vertexesBelowSurface |= 1 << i;
		}
	}

	const int* tris = triTable[vertexesBelowSurface];

	for (int16_t i{0}; i < 16; ++i) {
		//end of vertexes TODO: could loop in 3's to prevent incomplete triangles being pushed (should never happen but who knows))
		if (tris[i] == -1){
			break;
		}
		outVertexes.emplace_back(edgeVertexes[tris[i]][0],edgeVertexes[tris[i]][1] ,edgeVertexes[tris[i]][2]);
	}
}

std::vector<glm::vec3> MarchingCubes::MarchCubes(const std::vector<std::vector<std::vector<float>>>& gridValues, float surfaceValue) {

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

				MarchCube(vertexes, cubeVals, surfaceValue);

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