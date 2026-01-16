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
        void setBool(const std::string &name, const bool &value) const;
        void setVec3(const std::string &name, const glm::vec3 &value) const;
        void setFloat(const std::string &name, const float &value) const;
};

#endif