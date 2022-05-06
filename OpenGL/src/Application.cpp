#pragma
/*In the preprocessor definitions (C/C++ -> preprocessing -> preprocessing definitions), define
  GLEW_STATIC which is a macro that evaluates whether glew is a dll or a static lib*/

#include <GL/glew.h> //Need to be included because of the macros in glew.h


#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

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
    unsigned int id = glCreateShader(type);
    //Point to the first character of the string source
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        /*Can't just allocate an array with a dynamic variable, it wants a constant. So a bit of wizardery to make it work.
        * Allocate the number of bytes needed for the array based on the size of a char and the array length, cast that 
          to a char pointer wich is basicaly the same thing as an array.
          NOTE: alloca is used to allocate memory dynamically on the stack*/
        char* message = (char*) alloca(length * sizeof(char));

        glGetShaderInfoLog(id, length, &length, message);
        std::string shaderType = (type == GL_VERTEX_SHADER) ? "vertex" : "fragment";
        std::cout << "Failed to compile " << shaderType << " shader" << std::endl;
        std::cout << message << std::endl;

        glDeleteShader(id);

        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    //Attach the 2 shaders to our program. This specifies that our shaders will be included in the linking of the program
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    //Links and validate the program
    glLinkProgram(program);
    glValidateProgram(program);

    /*We can delete the shaders since the program is an intermediate file that executes our shaders.
      Technically we should use glDetachShader, after the linking but since it deletes the source code and that we migth need
      it for debugging purposes, glDeleteShader will do for our needs */
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);



    //Verify that glew init succeeds, which makes the link between the openGL implementation of the hardware and the standard function calls.
    if (glewInit() != GLEW_OK) {
        std::cout << "Error! glew init went wrong." << std::endl;
    }
    else {
        std::cout << glGetString(GL_VERSION) << std::endl;
    }

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

    /****************************************************************
    =============================VBO=================================
    *****************************************************************/

    //Assigns an id for the buffer
    unsigned int buffer;
    glGenBuffers(1, &buffer);

    //Select the buffer as active(binding)
    //GL_ARRAY_BUFFER is to precise usage of the buffer
    glBindBuffer(GL_ARRAY_BUFFER, buffer);

    /*The target to wich the buffer is bound (GL_ARRAY_BUFFER which is a buffer for vertex attributes),
    how big does the buffer have to be(in bytes), the source and usage of the data*/
    glBufferData(GL_ARRAY_BUFFER, 3 * 4 * sizeof(float), &positions, GL_STATIC_DRAW);

    //Enables the vertex attribute array that we define just below
    glEnableVertexAttribArray(0); //index of the vertex we want to enable

    /*For each vertex, we only have 1 attribute: a position, so bind it to the index 0 and only neew to
      call the function once.*/
    glVertexAttribPointer(0, //index at which we can find the first position
                          3, //number of elements that make a position
                          GL_FLOAT, //Type of the elements that constitutes our attribute
                          GL_FALSE, //whether we want them to be normalized or not by openGL
                          3 * sizeof(float), //How many bytes is between 2 vertices, the stride.
                          0 // pointer to the next attribute. there is none so we can put 0.
                          );

    /****************************************************************
    =============================IBO=================================
    *****************************************************************/

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 3 * 2 * sizeof(unsigned int), &indices, GL_STATIC_DRAW);


    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");

    std::string vertexShader = source.VertexSource;
    std::string fragmentShader = source.FragmentSource;

    unsigned int shader = CreateShader(vertexShader, fragmentShader);
    glUseProgram(shader);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}