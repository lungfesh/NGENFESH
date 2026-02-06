#ifndef TEXTURE_HPP
#define TEXTURE_HPP

#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class Texture {
    public:
        unsigned int texture = 0; // make texture object
        void init(std::string textureFile);
        ~Texture();

        void use() const;
        void unUse() const;
};

#endif