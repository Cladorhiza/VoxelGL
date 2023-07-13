//stl
#include <iostream>
#include <vector>
#include <cstdlib>
#include <array>
#include <time.h>
//thirdparty
#include "GL/glew.h"
#include "gl/GL.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
//my headers

#include "InputManager.h"
#include "Shader.h"
#include "Camera.h"
#include "GLUtil.h"
#include "MarchingCubes.h"
//globals
GLFWwindow* g_window{nullptr};

glm::mat4 projection{glm::perspective(45.0f, 16.0f/9.0f,0.01f,1000.0f)};
glm::mat4 view{Camera::GetViewMatrix()};

float getfunc(int x, int y, int z) {
	return x*x + y*y + z*z;
}

int Init() {

    std::cout << "Initializing g_window and openGL.\n";

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    /* Create a windowed mode g_window and its OpenGL context */
    g_window = glfwCreateWindow(1920/2, 1080/2, "marching cubes", NULL, NULL);

	if (!g_window) {
        glfwTerminate();
        return -1;
    }

    /* Make the g_window's context current */
    glfwMakeContextCurrent(g_window);

    if (glewInit() != GLEW_OK) {
		std::cout << "openGL failed to initialize" << '\n';
	}

	InputManager::Init(g_window);
    return 1;
}

int main() {

    if (Init() < 0) {
	    std::cout << "Initialization failed! rip." << std::endl;
        return -1;
    }
	std::cout << "Initialization successful!" << std::endl;
    

    Shader shader("res/shaders/BasicShader.txt");
    shader.Bind();
	shader.SetUniformMat4f("projectionMatrix", projection);
	shader.SetUniformMat4f("viewMatrix", view);

    std::vector<std::vector<std::vector<float>>> cubeData;

	constexpr uint32_t gridCount{40};
	time_t t;
	srand(static_cast<unsigned>(time(&t)));

    for (int i{0}; i < gridCount; ++i) {
        cubeData.emplace_back();
	    for (int j{0}; j < gridCount; ++j) {
			cubeData[i].emplace_back();
            for (int k{0}; k < gridCount; ++k) {
                cubeData[i][j].push_back(getfunc(i,j,k));

            }
	    }
    }

	float surfaceLevel = 1600;

	std::vector<glm::vec3> marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel);

	std::vector<unsigned> marchingIndexes;
	std::vector<glm::vec3> marchingColours;

	for (size_t i{0}; i+3 < marchingVertexes.size(); i+=3) {

		marchingIndexes.push_back(i);
		marchingIndexes.push_back(i+1);
		marchingIndexes.push_back(i+2);
		marchingColours.emplace_back(1.0f, 0.0f, 0.0f);
		marchingColours.emplace_back(0.0f, 1.0f, 0.0f);
		marchingColours.emplace_back(0.0f, 0.0f, 1.0f);
	}

	uint32_t vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes);
    
	/* Loop until the user closes the g_window */
    while (!glfwWindowShouldClose(g_window))
    {
        //Rendering
        glClear(GL_COLOR_BUFFER_BIT);

		shader.SetUniformMat4f("viewMatrix",Camera::GetViewMatrix());

		glBindVertexArray(vaoMarch);
    	glDrawElements(GL_TRIANGLES, marchingIndexes.size(), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
        /* Swap front and back buffers */
        glfwSwapBuffers(g_window);

        /* Poll for and process events */
        glfwPollEvents();
        //Update inputs
        InputManager::Poll(g_window);

        if (InputManager::GetKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
	        break;
        }
    	if (InputManager::GetKeyToggle(GLFW_KEY_A) && InputManager::GetKeyState(GLFW_KEY_A) == GLFW_PRESS) {
	        surfaceLevel += 10.0f;
            marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel);
            glDeleteVertexArrays(1, &vaoMarch);
            vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes);
        }
    	if (InputManager::GetKeyToggle(GLFW_KEY_D) && InputManager::GetKeyState(GLFW_KEY_D) == GLFW_PRESS) {
	        surfaceLevel -= 10.0f;
            marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel);
            glDeleteVertexArrays(1, &vaoMarch);
            vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes);
        }
		Camera::Update();
    }

    glfwTerminate();
    return 0;

    
}