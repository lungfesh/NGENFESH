#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
    public:
        unsigned int ID;
        Shader(std::string vertexShaderFile, std::string fragmentShaderFile);
        void use();
        void setMat4(const std::string &name, const glm::mat4 &value) const;
};

#endif