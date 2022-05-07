#pragma once
#include <GL/glew.h>

#ifdef OGL_DEBUG == 1
#define LOG(x) std::cout << x << std::endl
#define ASSERT(y) if (!(y)) __debugbreak();
#define GLCall(x) GLClearError();\
        x;\
        ASSERT(GLLogCall(#x, __FILE__, __LINE__ ))
#elif defined(OGL_RELEASE)
#define GLCall(x) x
#define LOG(x)
#else
#define LOG(x)
#endif


void AdjustIncrement(float& increment, float component);
void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);