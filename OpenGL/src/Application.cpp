#pragma
/*In the preprocessor definitions (C/C++ -> preprocessing -> preprocessing definitions), define
  GLEW_STATIC which is a macro that evaluates whether glew is a dll or a static lib*/

#include <GL/glew.h> //Need to be included because of the macros in glew.h
#include "Renderer.h"

#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexArray.h"


struct ShaderProgramSource{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath) 
{
    std::ifstream stream(filepath);

    enum class ShaderType { NONE = -1, VERTEX = 0, FRAGMENT = 1};

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) {

            if (line.find("vertex") != std::string::npos) {
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << std::endl;
        }
    }

    return { ss[0].str(), ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    GLCall(unsigned int id = glCreateShader(type));
    //Point to the first character of the string source
    const char* src = source.c_str();
    GLCall(glShaderSource(id, 1, &src, nullptr));
    GLCall(glCompileShader(id));

    int result;
    GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));

    if (result == GL_FALSE) {
        int length;
        GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
        /*Can't just allocate an array with a dynamic variable, it wants a constant. So a bit of wizardery to make it work.
        * Allocate the number of bytes needed for the array based on the size of a char and the array length, cast that 
          to a char pointer wich is basicaly the same thing as an array.
          NOTE: alloca is used to allocate memory dynamically on the stack*/
        char* message = (char*) alloca(length * sizeof(char));

        GLCall(glGetShaderInfoLog(id, length, &length, message));
        std::string shaderType = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        std::cout << "Failed to compile " << shaderType << " shader" << std::endl;
        std::cout << message << std::endl;

        GLCall(glDeleteShader(id));

        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string fragmentShader)
{
    GLCall(unsigned int program = glCreateProgram());
    GLCall(unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader));
    GLCall(unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader));

    //Attach the 2 shaders to our program. This specifies that our shaders will be included in the linking of the program
    GLCall(glAttachShader(program, vs));
    GLCall(glAttachShader(program, fs));
    //Links and validate the program
    GLCall(glLinkProgram(program));
    GLCall(glValidateProgram(program));

    /*We can delete the shaders since the program is an intermediate file that executes our shaders.
      Technically we should use glDetachShader, after the linking but since it deletes the source code and that we migth need
      it for debugging purposes, glDeleteShader will do for our needs */
    GLCall(glDeleteShader(vs));
    GLCall(glDeleteShader(fs));

    return program;
}

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
           -0.5f, -0.5f, 0.0f,
            0.5f, -0.5f, 0.0f,
            0.5f,  0.5f, 0.0f,
           -0.5f,  0.5f, 0.0f

        };

        //Sequence of indices of vertices to draw each triangles that makes up the square
        unsigned int indices[] = {
            0, 1, 2,
            2, 3, 0
        };

        VertexArray va;

        VertexBuffer vb(positions, 3 * 4 * sizeof(float));

        VertexBufferLayout layout;
        layout.Push<float>(3);
        va.AddBuffer(vb, layout);

        IndexBuffer ib(indices, 6);

        /****************************************************************
        =============================SHADER==============================
        *****************************************************************/


        ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

        std::string vertexShader = source.VertexSource;
        std::string fragmentShader = source.FragmentSource;

        unsigned int shader = CreateShader(vertexShader, fragmentShader);
        GLCall(glUseProgram(shader));

        //After the shader is boud, the location of u_Color can be retrieved
        GLCall(int location = glGetUniformLocation(shader, "u_Color"));
        //-1 means the uniform couldn't be found in the sahder
        ASSERT(location != -1);


        float r = 0.8f;
        float g = 0.2f;
        float b = 0.8f;
        float increment_r = 0.02f;
        float increment_b = -0.02f;
        float increment_g = 0.01f;

        /****************************************************************
        ============================RENDERING============================
        *****************************************************************/
        while (!glfwWindowShouldClose(window))
        {
            /* Render here */
            GLCall(glClear(GL_COLOR_BUFFER_BIT));


            GLCall(glUseProgram(shader));
            //Giving the location and value, we are setting the uniform
            GLCall(glUniform4f(location, r, g, b, 1.0f));

           va.Bind();
            ib.Bind();

            //when we only had a vbo, the call was glDrafArrays(GL_TRIANGLES, 0, 6)
            GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));

            AdjustIncrement(*&increment_r, r);
            AdjustIncrement(*&increment_g, g);
            AdjustIncrement(*&increment_b, b);

            r += increment_r;
            g += increment_g;
            b += increment_b;

            /* Swap front and back buffers */
            glfwSwapBuffers(window);

            /* Poll for and process events */
            glfwPollEvents();
        }
        GLCall(glDeleteProgram(shader));
    }


    glfwTerminate();
    return 0;
}