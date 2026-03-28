#include "shader_def.hpp"
#include "shader.hpp"


Shader* objectShader = nullptr;
Shader* lightShader  = nullptr;
Shader* debugShader  = nullptr;

// needs to be called after window is initalized, because Shader uses some opengl functions
void initShaders() {
    objectShader = new Shader("shaders/object.vert", "shaders/object.frag");
    lightShader  = new Shader("shaders/light.vert", "shaders/light.frag");
    debugShader  = new Shader("shaders/debug.vert", "shaders/debug.frag");
}