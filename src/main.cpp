//stl
#include <iostream>
#include <vector>
#include <cstdlib>
#include <array>
#include <time.h>
#include <deque>
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
constexpr uint32_t CHONK_SIZE { 32 };
constexpr float CHUNK_LOAD_RADIUS { 160.f };
constexpr float SURFACE_LEVEL = 0.f;

//if radius / chunkSize != an integer, this won't look at correctly spaced chunks
std::vector<glm::vec3> GetChunksWithinRadiusFromPoint(glm::vec3 point, float radius, float chunkSize){

	//truncate the point to a chunk point
	point /= chunkSize;
	glm::ivec3 intPoint {point};
	point = glm::vec3{intPoint} * chunkSize;

	std::vector<glm::vec3> chunkLocationsToLoad;


	glm::vec3 minBound {point - radius};
	glm::vec3 maxBound {point + radius};

	for (float x {minBound.x}; x <= maxBound.x; x+= chunkSize){
		for (float y {minBound.y}; y <= maxBound.y; y+= chunkSize){
			for (float z {minBound.z}; z <= maxBound.z; z+= chunkSize){
			
				if (glm::length(glm::vec3{x, y, z} - point) < radius){
					chunkLocationsToLoad.emplace_back(x, y, z);
				}
			}
		}
	}
	return chunkLocationsToLoad;
} 

struct Chonk {

	std::vector<std::vector<std::vector<float>>> density;

	glm::vec3 position;
};

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
	glEnable(GL_DEPTH_TEST);
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
	
	time_t t;
	time(&t);
	srand(t);
	fn.SetSeed(rand());

    std::vector<std::vector<std::vector<float>>> cubeData;

	std::vector<Chonk> loadedChunks;
	std::vector<glm::vec3> chunkLoadLocations { GetChunksWithinRadiusFromPoint({0.f, 0.f, 0.f}, CHUNK_LOAD_RADIUS, CHONK_SIZE) };
	std::deque<glm::vec3> chunksToLoad { chunkLoadLocations.begin(), chunkLoadLocations.end() };
	loadedChunks.reserve(chunksToLoad.size());
	std::vector<std::pair<unsigned, size_t>> glChunkIDs;
	glChunkIDs.reserve(chunksToLoad.size());

	for (const glm::vec3& chunkPos : chunksToLoad){
		
		loadedChunks.emplace_back();

		//TODO: The chunks are generating one larger because they don't share vertices yet
		Chonk& currentChunk = loadedChunks.back();
		currentChunk.position = chunkPos;
		currentChunk.density.reserve(CHONK_SIZE+1);
		
		for (int x { 0 }; x < CHONK_SIZE+1; x++){
			currentChunk.density.emplace_back();
			currentChunk.density[x].reserve(CHONK_SIZE+1);
			for (int y { 0 }; y < CHONK_SIZE+1; y++){
				currentChunk.density[x].emplace_back();
				currentChunk.density[x][y].resize(CHONK_SIZE+1);
				for (int z { 0 }; z < CHONK_SIZE+1; z++){
				
					currentChunk.density[x][y][z] = fn.GetNoise(static_cast<float>(x + static_cast<int>(chunkPos.x)),static_cast<float>(y + static_cast<int>(chunkPos.y)),static_cast<float>(z + static_cast<int>(chunkPos.z)));
					
				}
			}
		}

		std::vector<glm::vec3> marchingNormals;
		std::vector<unsigned>  marchingIndexes;
		std::vector<glm::vec3> marchingColours;
		std::vector<glm::vec3> marchingVertexes = MarchingCubes::MarchCubes(currentChunk.density, surfaceLevel, marchingNormals, chunkPos);

		for (size_t i{0}; i+3 < marchingVertexes.size(); i+=3) {
			marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
			marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
			marchingColours.emplace_back(1.0f, 1.0f, 1.0f);
			marchingIndexes.emplace_back(i);
			marchingIndexes.emplace_back(i+1);
			marchingIndexes.emplace_back(i+2);
		}

		uint32_t vaoMarch = GLUtil::BuildVAOfromData(marchingVertexes, marchingColours, marchingIndexes, marchingNormals);
		glChunkIDs.emplace_back(vaoMarch, marchingIndexes.size());
	}

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

		Camera::Update();

        //Rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Light Settings");
        ImGui::SliderFloat3("Light WorldPosition", &lightPosition.x, -1000.0f, 1000.0f);
		shader.SetUniformvec3f("lightWorldPosition", lightPosition.x, lightPosition.y, lightPosition.z);
        //ImGui::ColorEdit3("Ambient Intensity", &shader.lightInfo.ambientIntensity.x);            
        //framerate
        //ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
        ImGui::End();
        

        ImGui::Render();

		shader.SetUniformMat4f("viewMatrix",Camera::GetViewMatrix());

		for (auto [vao, indexCount] : glChunkIDs){
		
			glBindVertexArray(vao);
    		glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, nullptr);
			glBindVertexArray(0);
		}
        
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