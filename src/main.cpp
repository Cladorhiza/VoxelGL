//stl
#include <iostream>
#include <vector>
//thirdparty
#include "GL/glew.h"
#include "gl/GL.h"
#include "GLFW/glfw3.h"
//my headers
#include "InputManager.h"
#include "Shader.h"

//globals
GLFWwindow* g_window{nullptr};



unsigned buildVAOfromData(const std::vector<float>& vertexes, 
                                const std::vector<float>& colours, 
                                const std::vector<unsigned>& indexes)
{
    //ids for buffer objects
    unsigned vbo[2], vao, ibo;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    //generate vertex buffers and assign vertexes to first
    glGenBuffers(2, vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
    glBufferData(GL_ARRAY_BUFFER, vertexes.size() * sizeof(float), vertexes.data(), GL_STATIC_DRAW);

    //shader layout location for vertexes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    //assign colours to second
    glBindBuffer(GL_ARRAY_BUFFER, vbo[1]);
    glBufferData(GL_ARRAY_BUFFER, colours.size() * sizeof(float), colours.data(), GL_STATIC_DRAW);

    //shader layout location for colours
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glEnableVertexAttribArray(1);

    //generate index buffer and assign indexes to it
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexes.size() * sizeof(unsigned), indexes.data(), GL_STATIC_DRAW);

    return vao;
}

int Init() {

    std::cout << "Initializing g_window and openGL.\n";

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }

    /* Create a windowed mode g_window and its OpenGL context */
    g_window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

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
    else {
	    std::cout << "Initialization successful!" << std::endl;
    }

    Shader shader("res/shaders/BasicShader.txt");
    shader.Bind();

    std::vector<float> vertexes {
	  0.0f, 0.75f, 0.0f,
	  0.5f, 0.0f, 0.0f,
	  -0.5f, 0.0f, 0.0f
    };

    std::vector<float> colours {
	  1.0f, 1.0f, 0.0f,  
	  0.0f, 1.0f, 1.0f,  
	  1.0f, 0.0f, 1.0f
    };

    std::vector<unsigned> indexes {
	    0,1,2
    };

    uint32_t vao = buildVAOfromData(vertexes, colours, indexes);
	glBindBuffer(GL_VERTEX_ARRAY, vao);

	/* Loop until the user closes the g_window */
    while (!glfwWindowShouldClose(g_window))
    {
        //Rendering
        glClear(GL_COLOR_BUFFER_BIT);

    	glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Swap front and back buffers */
        glfwSwapBuffers(g_window);

        /* Poll for and process events */
        glfwPollEvents();
        //Update inputs
        InputManager::Poll(g_window);

        if (InputManager::GetKeyState(GLFW_KEY_ESCAPE) == GLFW_PRESS) {
	        break;
        }
    }

    glfwTerminate();
    return 0;

    
}