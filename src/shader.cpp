#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
Shader::Shader(std::string vertexShaderFile, std::string fragmentShaderFile) {
    std::string vertexShaderCode;
    std::string fragmentShaderCode;
    std::ifstream vFile(vertexShaderFile);
    std::ifstream fFile(fragmentShaderFile);
    if (!vFile) {
        std::cout << "Cannot read " << vertexShaderFile << std::endl;
        return;
    }
    std::stringstream vBuffer;
    std::stringstream fBuffer;
    vBuffer << vFile.rdbuf();
    vertexShaderCode = vBuffer.str();
    const char* vertexShaderSource = vertexShaderCode.c_str();
    
    
    if (!fFile) {
        std::cout << "Cannot read " << fragmentShaderFile << std::endl;
        return;
    }
    fBuffer << fFile.rdbuf();
    fragmentShaderCode = fBuffer.str();
    const char* fragmentShaderSource = fragmentShaderCode.c_str();
    // compile vertexShader
    int success;
    char infoLog[512];
    
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
    }
    // compile fragmentShader 
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(fragmentShader,512,NULL,infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT:COMPILATION_FAILED\n" << infoLog << "\n";
    }
    // shader Program
    ID = glCreateProgram();
    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);
    // print linking errors if any
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if(!success)
    {
        glGetProgramInfoLog(ID, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    // delete the shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}
void Shader::use() {
    glUseProgram(ID);
}

void Shader::setMat4(const std::string &name, const glm::mat4 &value) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
}
void Shader::setBool(const std::string &name, const bool &value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}