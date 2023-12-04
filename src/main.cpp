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
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "imgui_impl_glfw.h"

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
constexpr uint32_t WIDTH { 1000 };
constexpr uint32_t HEIGHT { 1000 };
constexpr float CLIP_NEAR {0.01f};
constexpr float CLIP_FAR {2000.f};
constexpr float VFOV { 70.0f };
constexpr uint32_t CHUNK_SIZE { 32 };
constexpr float SURFACE_LEVEL = 1000.f;

int Init() {

    std::cout << "Initializing g_window and openGL.\n";

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }
	 
    /* Create a windowed mode g_window and its OpenGL context */
    g_window = glfwCreateWindow(WIDTH, HEIGHT, "Marching Cubes", NULL, NULL);

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

	glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

	//imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    const char* glsl_version = "#version 150";
    ImGui_ImplOpenGL3_Init(glsl_version);

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

	glm::mat4 projection{ glm::perspective(glm::radians(VFOV), static_cast<float>(WIDTH)/HEIGHT, CLIP_NEAR, CLIP_FAR) };
	glm::mat4 view{Camera::GetViewMatrix()};
	float surfaceLevel = SURFACE_LEVEL;

    Shader shader("res/shaders/BasicShader.txt");
    shader.Bind();
	shader.SetUniformMat4f("projectionMatrix", projection);
	shader.SetUniformMat4f("viewMatrix", view);

    FastNoise fn;
    fn.SetNoiseType(FastNoise::PerlinFractal);

    std::vector<std::vector<std::vector<float>>> cubeData;

    for (int i{0}; i < CHUNK_SIZE; ++i) {
        cubeData.emplace_back();
	    for (int j{0}; j < CHUNK_SIZE; ++j) {
			cubeData[i].emplace_back();
            for (int k{0}; k < CHUNK_SIZE; ++k) {
                //cubeData[i][j].push_back(fn.GetNoise(static_cast<float>(i),static_cast<float>(j),static_cast<float>(k)));
                cubeData[i][j].push_back(sphereFunc(static_cast<float>(i),static_cast<float>(j),static_cast<float>(k)));

            }
	    }
    }

	std::vector<glm::vec3> marchingNormals;
	std::vector<unsigned>  marchingIndexes;
	std::vector<glm::vec3> marchingColours;
	std::vector<glm::vec3> marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel, marchingNormals);

	for (size_t i{0}; i+3 < marchingVertexes.size(); i+=3) {
		marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
		marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
		marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
		marchingIndexes.emplace_back(i);
		marchingIndexes.emplace_back(i+1);
		marchingIndexes.emplace_back(i+2);
	}

	uint32_t vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes, marchingNormals);

	glm::vec3 lightPosition { 100.0f, 0.0f, 0.0f };
	shader.SetUniformvec3f("lightWorldPosition", lightPosition.x, lightPosition.y, lightPosition.z);

	/* Loop until the user closes the g_window */
    while (!glfwWindowShouldClose(g_window))
    {
		/* Poll for and process events */
        InputManager::Poll(g_window);

        if (InputManager::GetKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
	        break;
        }
		//regenerate chunk with increased surface level
    	if (InputManager::GetKeyState(GLFW_KEY_A) == GLFW_PRESS) {
	        surfaceLevel += 5.f;
            std::cout << surfaceLevel << '\n';

			marchingNormals.clear();
            marchingVertexes.clear();
			marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel, marchingNormals);
            glDeleteVertexArrays(1, &vaoMarch);
            vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes, marchingNormals);
        }
		//regenerate chunk with decreased surface level
    	if (InputManager::GetKeyState(GLFW_KEY_D) == GLFW_PRESS) {
	        surfaceLevel -= 5.f;
            std::cout << surfaceLevel << '\n';

			marchingNormals.clear();
            marchingVertexes.clear();
            marchingVertexes = MarchingCubes::MarchCubes(cubeData, surfaceLevel, marchingNormals);
            glDeleteVertexArrays(1, &vaoMarch);
            vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes, marchingNormals);
        }
		if (InputManager::GetKeyState(GLFW_KEY_D) == GLFW_PRESS || InputManager::GetKeyState(GLFW_KEY_A) == GLFW_PRESS) {
		
			marchingColours.clear();
			marchingIndexes.clear();
			for (size_t i{0}; i+3 < marchingVertexes.size(); i+=3) {
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
				marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
				marchingIndexes.emplace_back(i);
				marchingIndexes.emplace_back(i+1);
				marchingIndexes.emplace_back(i+2);
			}
		}

		Camera::Update();

        //Rendering
        glClear(GL_COLOR_BUFFER_BIT);

		// Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Light Settings");
        ImGui::SliderFloat3("Light WorldPosition", &lightPosition.x, -100.0f, 100.0f);
		shader.SetUniformvec3f("lightWorldPosition", lightPosition.x, lightPosition.y, lightPosition.z);
        //ImGui::ColorEdit3("Ambient Intensity", &shader.lightInfo.ambientIntensity.x);            
        //framerate
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
        

        ImGui::Render();

		shader.SetUniformMat4f("viewMatrix",Camera::GetViewMatrix());

		glBindVertexArray(vaoMarch);
    	glDrawElements(GL_TRIANGLES, static_cast<unsigned>(marchingIndexes.size()), GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
        
		//draw UI over everything else
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        
		/* Swap front and back buffers */
        glfwSwapBuffers(g_window);
    }

    glfwTerminate();
    return 0;
}

int ComputeTest(){
    
	if (Init() < 0) {
	    std::cout << "Initialization failed! rip." << '\n';
	    return -1;
	}
	std::cout << "Initialization successful!" << '\n';

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

		float currentFrame = static_cast<float>(glfwGetTime());
		shader.Bind();
		shader.SetUniform1f("t", currentFrame);
		shader.Unbind();
	}

	glfwTerminate();
	return 0;
}

void PrintGLStats(){

	int numAttributes = 0;
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
    std::cout << "Max vertex attributes: " << numAttributes << '\n';

    glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &numAttributes);
    std::cout << "Max uniform locations: " << numAttributes << '\n';

    glGetIntegerv(GL_MAJOR_VERSION, &numAttributes);
    std::cout << "GL version: " << numAttributes << '.';
    glGetIntegerv(GL_MINOR_VERSION, &numAttributes);
    std::cout << numAttributes << '\n';

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &numAttributes);
	std::cout << "Max vertex attributes: " << numAttributes << '\n';

	glGetIntegerv(GL_MAX_UNIFORM_LOCATIONS, &numAttributes);
	std::cout << "Max uniform locations: " << numAttributes << '\n';

	glGetIntegerv(GL_MAJOR_VERSION, &numAttributes);
	std::cout << "GL version: " << numAttributes << '.';
	glGetIntegerv(GL_MINOR_VERSION, &numAttributes);
	std::cout << numAttributes << '\n';

	int computeCount[]{0,0,0};
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, computeCount);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, computeCount + 1);
	glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, computeCount + 2);
	std::cout << "max X compute: " << computeCount[0] <<
		        ".max Y compute: " << computeCount[1] <<
		        ".max Z compute: " << computeCount[2] << '\n';

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &numAttributes);
	std::cout << "max shader dispatch count: " << numAttributes << '\n';

	glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_SIZE , &numAttributes);
	std::cout << "max work group size: " << numAttributes << '\n';

}

int main() {
	cubeMarch();
}