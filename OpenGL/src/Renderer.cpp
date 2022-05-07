#include "Renderer.h"
#include <iostream>

void AdjustIncrement(float& increment, float component) {
    if (component > 1.0f)
        increment = -increment;
    else if (component < 0.0f)
        increment = -increment;
}

void GLClearError() {
    while (glGetError() != GL_NO_ERROR);
}

bool GLLogCall(const char* function, const char* file, int line) {
    while (GLenum error = glGetError()) {
        std::cout << "[OpenGl Error] (" << error << ") " << function
            << " " << file << ": " << line << std::endl;
        return false;
    }

    return true;
}