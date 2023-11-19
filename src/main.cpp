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
#include "FastNoise.h"
#include "ComputeShader.h"
#include "Stopwatch.h"

//globals
GLFWwindow* g_window{nullptr};

glm::mat4 projection{glm::perspective(45.0f, 16.0f/9.0f,0.01f,1000.0f)};
glm::mat4 view{Camera::GetViewMatrix()};

int Init() {

    std::cout << "Initializing g_window and openGL.\n";

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    /* Create a windowed mode g_window and its OpenGL context */
    g_window = glfwCreateWindow(1000, 1000, "marching cubes", NULL, NULL);

	if (!g_window) {
        glfwTerminate();
        return -1;
    }

    /* Make the g_window's context current */
    glfwMakeContextCurrent(g_window);

	glfwSwapInterval(0); /*disable vsync*/

    if (glewInit() != GLEW_OK) {
		std::cout << "openGL failed to initialize" << '\n';
	}

	InputManager::Init(g_window);
    return 1;
}

float sphereFunc(float x, float y, float z) {
	return x * x + y * y + z * z;
}

int cubeMarch() {
	if (Init() < 0) {
	    std::cout << "Initialization failed! rip." << '\n';
        return -1;
    }
	std::cout << "Initialization successful!" << '\n';
    int numAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
    std::cout << "Max vertex attributes: " << numAttributes << '\n';

    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &numAttributes);
    std::cout << "Max uniform locations: " << numAttributes << '\n';

    glGetIntegerv(GL_MAJOR_VERSION, &numAttributes);
    std::cout << "GL version: " << numAttributes << '.';
    glGetIntegerv(GL_MINOR_VERSION, &numAttributes);
    std::cout << numAttributes << '\n';



    Shader shader("res/shaders/BasicShader.txt");
    shader.Bind();
	shader.SetUniformMat4f("projectionMatrix", projection);
	shader.SetUniformMat4f("viewMatrix", view);

    FastNoise fn;
    fn.SetNoiseType(FastNoise::PerlinFractal);



    std::vector<std::vector<std::vector<float>>> cubeData;

	constexpr uint32_t gridCount{50};
	time_t t;
	srand(static_cast<unsigned>(time(&t)));

    for (int i{0}; i < gridCount; ++i) {
        cubeData.emplace_back();
	    for (int j{0}; j < gridCount; ++j) {
			cubeData[i].emplace_back();
            for (int k{0}; k < gridCount; ++k) {
                //cubeData[i][j].push_back(fn.GetNoise(static_cast<float>(i),static_cast<float>(j),static_cast<float>(k)));
                cubeData[i][j].push_back(sphereFunc(static_cast<float>(i),static_cast<float>(j),static_cast<float>(k)));

            }
	    }
    }

    float surfaceLevel = 3000.0f;

	std::vector<glm::vec3> marchingNormals;
	std::vector<glm::vec3> marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel, marchingNormals);

	std::vector<unsigned> marchingIndexes;
	std::vector<glm::vec3> marchingColours;

	for (size_t i{0}; i+3 < marchingVertexes.size(); i+=3) {

		marchingIndexes.push_back(static_cast<unsigned>(i));
		marchingIndexes.push_back(static_cast<unsigned>(i)+1);
		marchingIndexes.push_back(static_cast<unsigned>(i)+2);
		marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
		marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
		marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
	}

	uint32_t vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes, marchingNormals);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	/* Loop until the user closes the g_window */
    while (!glfwWindowShouldClose(g_window))
    {
		/* Poll for and process events */
        InputManager::Poll(g_window);

        if (InputManager::GetKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
	        break;
        }
    	if (InputManager::GetKeyState(GLFW_KEY_A) == GLFW_PRESS) {
	        surfaceLevel += 1.f;
            std::cout << surfaceLevel << '\n';
            marchingVertexes.clear();
            marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel, marchingNormals);
            glDeleteVertexArrays(1, &vaoMarch);
            vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes, marchingNormals);

            marchingColours.clear();
            marchingIndexes.clear();
            for (size_t i{0}; i+3 < marchingVertexes.size(); i+=3) {

				marchingIndexes.push_back(static_cast<unsigned>(i));
				marchingIndexes.push_back(static_cast<unsigned>(i)+1);
				marchingIndexes.push_back(static_cast<unsigned>(i)+2);
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
			}
        }
    	if (InputManager::GetKeyState(GLFW_KEY_D) == GLFW_PRESS) {
	        surfaceLevel -= 1.f;
            std::cout << surfaceLevel << '\n';
            marchingVertexes.clear();
            marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel, marchingNormals);
            glDeleteVertexArrays(1, &vaoMarch);
            vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes, marchingNormals);

            marchingColours.clear();
            marchingIndexes.clear();
            for (size_t i{0}; i+3 < marchingVertexes.size(); i+=3) {

				marchingIndexes.push_back(static_cast<unsigned>(i));
				marchingIndexes.push_back(static_cast<unsigned>(i)+1);
				marchingIndexes.push_back(static_cast<unsigned>(i)+2);
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
			}

        }
		Camera::Update();

        //Rendering
        glClear(GL_COLOR_BUFFER_BIT);

		shader.SetUniformMat4f("viewMatrix",Camera::GetViewMatrix());

		glBindVertexArray(vaoMarch);
    	glDrawElements(GL_TRIANGLES, static_cast<unsigned>(marchingIndexes.size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
        
        /* Swap front and back buffers */
        glfwSwapBuffers(g_window);

    }

    glfwTerminate();
    return 0;

    
}

int main() {

	if (Init() < 0) {
	    std::cout << "Initialization failed! rip." << '\n';
	    return -1;
	}

	std::cout << "Initialization successful!" << '\n';
	int numAttributes = 0;
	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
	std::cout << "Max vertex attributes: " << numAttributes << '\n';

	glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &numAttributes);
	std::cout << "Max uniform locations: " << numAttributes << '\n';

	glGetIntegerv(GL_MAJOR_VERSION, &numAttributes);
	std::cout << "GL version: " << numAttributes << '.';
	glGetIntegerv(GL_MINOR_VERSION, &numAttributes);
	std::cout << numAttributes << '\n';

	int computeCount[3]{0,0,0};
	glGetIntegeri_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, computeCount);
	glGetIntegeri_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, computeCount+1);
	glGetIntegeri_v (GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, computeCount+2);
	std::cout << "max X compute: " << computeCount[0] <<
		        ".max Y compute: " << computeCount[1] <<
		        ".max Z compute: " << computeCount[2] << '\n';

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &numAttributes);
	std::cout << "max shader dispatch count: " << numAttributes << '\n';

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_SIZE , &numAttributes);
	std::cout << "max work group size: " << numAttributes << '\n';

	std::vector<glm::vec3> vertexes {
	    {-1.f, 1.f, 0.0f},
	    { 1.f, 1.f, 0.0f},
	    { 1.f,-1.f, 0.0f},
	    { 1.f,-1.f, 0.0f},
	    {-1.f,-1.f, 0.0f},
	    {-1.f, 1.f, 0.0f}
	};

	std::vector<glm::vec2> texCoords {
	    {0.0f, 0.0f},
	    {1.0f, 0.0f},
	    {1.0f, 1.0f},
	    {1.0f, 1.0f},
	    {0.0f, 1.0f},
	    {0.0f, 0.0f}
	};

	unsigned int texturedQuad = GLUtil::BuildVAOfromData(vertexes, texCoords);

	ComputeShader shader("res/shaders/ComputeShader.txt");
	Shader texShader("res/shaders/TextureShader.txt");

	// texture size
	const unsigned int TEXTURE_WIDTH = 1000, TEXTURE_HEIGHT = 1000;
	unsigned int texture;

	glGenTextures(1, &texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, TEXTURE_WIDTH, TEXTURE_HEIGHT, 0, GL_RGBA, 
	             GL_FLOAT, NULL);

	glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

	float deltaTime { 0.0f };
	float lastFrame { 0.0f };
	int fCounter { 0 };

	Stopwatch stopwatch;

	//* Loop until the user closes the g_window */
	while (!glfwWindowShouldClose(g_window))
	{
	    //Rendering
	    glClear(GL_COLOR_BUFFER_BIT);

	    stopwatch.Restart();

	    shader.Bind();
	    glDispatchCompute(TEXTURE_WIDTH/25, TEXTURE_HEIGHT/25, 1);
	    shader.Unbind();

	    // make sure writing to image has finished before read
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
		
		std::cout << stopwatch << "\n";

	    glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

	    texShader.Bind();
	    glBindVertexArray(texturedQuad);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	    glBindVertexArray(0);
	    texShader.Unbind();

	    /* Swap front and back buffers */
	    glfwSwapBuffers(g_window);

	    /* Poll for and process events */
	    glfwPollEvents();
	    //Update inputs
	    InputManager::Poll(g_window);

	    if (InputManager::GetKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
	       break;
	    }
		Camera::Update();

		float currentFrame = glfwGetTime();
		shader.Bind();
		shader.SetUniform1f("t", currentFrame);
		shader.Unbind();
		
		//deltaTime += currentFrame - lastFrame;
		//lastFrame = currentFrame;
		//if(fCounter > 1000) {
		//	std::cout << "FPS: " << 1000 / deltaTime << std::endl;
		//	fCounter = 0;
		//	deltaTime = 0;
		//} else {
		//	fCounter++;
		//}
	}

	glfwTerminate();
	return 0;

    
}