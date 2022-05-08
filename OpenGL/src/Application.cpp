#pragma
/*In the preprocessor definitions (C/C++ -> preprocessing -> preprocessing definitions), define
  GLEW_STATIC which is a macro that evaluates whether glew is a dll or a static lib*/

#include <GL/glew.h> //Need to be included because of the macros in glew.h
#include "Renderer.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"



int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);


    glfwSwapInterval(1);

    //Verify that glew init succeeds, which makes the link between the openGL implementation of the hardware and the standard function calls.
    if (glewInit() != GLEW_OK) {
        LOG("Error! glew init went wrong.");
    }
    else {
        LOG(glGetString(GL_VERSION));
    }
    {
        //Vertices position for our triangle
        float positions[] = {
           -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
            0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
           -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
        };

        //Sequence of indices of vertices to draw each triangles that makes up the square
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        VertexArray va;

        VertexBuffer vb(positions, 5 * 4 * sizeof(float));

        VertexBufferLayout layout;
        layout.Push<float>(3);
        layout.Push<float>(2);
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        glm::mat4 proj = glm::ortho(-2.0f, 2.0f, -1.5f, 1.5f, -1.0f, 1.0f);

        Shader shader("res/shaders/Basic.shader");
        shader.Bind();

        Texture texture("res/textures/clouds.png");
        texture.Bind();

        shader.SetUniform1i("u_Texture", 0);
        shader.SetUniformMat4f("u_MVP", proj);

        va.Unbind();
        vb.Unbind();
        ib.Unbind();
        shader.Unbind();

        /****************************************************************
        ============================RENDERING============================
        *****************************************************************/

        Renderer renderer;
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */

            renderer.Clear();
            //Giving the location and value, we are setting the uniform
            shader.Bind();

            renderer.Draw(va, ib, shader);

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
    }


    glfwTerminate();
    return 0;
}