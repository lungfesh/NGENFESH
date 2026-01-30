#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
// #include "lighting.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

float windowWidth = 512.0f;
float windowHeight = 512.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = (float)width;
    windowHeight = (float)height;
} // called each time window is resized, and sets viewport size

float lastMouseX = windowWidth/2, lastMouseY = windowHeight/2;
float yaw;
float pitch;
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.9f)
        pitch = 89.9f;
    if (pitch < -89.9f)
        pitch = -89.9f;
}

glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
glm::vec3 forward;
glm::vec3 right;
bool fpsMovement = false;

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float cameraSpeed = 2.5f * deltaTime;

    if (fpsMovement){
        forward.x = cos(glm::radians(yaw));
        forward.y = 0.0f;
        forward.z = sin(glm::radians(yaw));
        forward = glm::normalize(forward);
    } else {
        forward = cameraFront;
    }

    right = glm::normalize(glm::cross(forward,cameraUp));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= right * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += right * cameraSpeed;
}

class Texture {
    public:
        unsigned int texture; // make texture object
        void init(std::string textureFile) {
            glGenTextures(1, &texture);
            glBindTexture(GL_TEXTURE_2D, texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            int width, height, nrChannels;
            unsigned char *data = stbi_load(textureFile.c_str(), &width, &height, &nrChannels, 0);
            if (data)
                {
                    GLenum format;
                if (nrChannels == 1) format = GL_RED;
                else if (nrChannels == 3) format = GL_RGB;
                else if (nrChannels == 4) format = GL_RGBA;
                else {
                    std::cout << "Unsupported texture format\n";
                    stbi_image_free(data);
                    return;
                }

                glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                glGenerateMipmap(GL_TEXTURE_2D);
            }
            else
            {
                std::cout << "Failed to load texture" << std::endl;
            }
            stbi_image_free(data);
        }
        ~Texture() {
            glDeleteTextures(1, &texture);
        }

        void use() const {
            glBindTexture(GL_TEXTURE_2D, texture);
        }
        void unUse() const {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
};

class Element {
    public:
        bool RENDERXYZ = true, RENDERCOLOR = true, RENDERTEX = true, RENDERNORM = true;
        unsigned int VAO, VBO, EBO;
        float x,y,z;
        bool wireframe = false;
        GLenum draw_mode = GL_TRIANGLES;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::string textureFile = "";
        bool useTexture = false;
        Texture texture;

        Shader* shader = nullptr;
        glm::vec3 lightColor;

        Element() = default;

        Element(const Element& other)
            : x(other.x), y(other.y), z(other.z),
              vertices(other.vertices), indices(other.indices) {
            init(); // create new VAO/VBO/EBO
        }

        void init() {
            if (useTexture)
                texture.init(textureFile);
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glGenBuffers(1, &EBO);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3*sizeof(float)));
            glEnableVertexAttribArray(1);
        
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6*sizeof(float)));
            glEnableVertexAttribArray(2);

            glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8*sizeof(float)));
            glEnableVertexAttribArray(3);
        };
        void draw(const glm::mat4& view, const glm::mat4& projection, Element& lightSource, glm::vec3 cameraPos) const {
            if (!shader) return;


            shader->use();
            shader->setMat4("view", view);
            shader->setMat4("projection", projection);
            shader->setMat4("model", getMatrix());
            shader->setBool("useTexture", getUseTexture());
            shader->setVec3("lightColor", lightSource.lightColor);
            shader->setVec3("lightPos", glm::vec3(lightSource.x, lightSource.y, lightSource.z));
            shader->setVec3("viewPos", cameraPos);
            if (wireframe)
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            if (useTexture)
                texture.use();
            glBindVertexArray(VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glDrawElements(draw_mode, indices.size(), GL_UNSIGNED_INT, 0);
            if (wireframe)
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
            if (useTexture)
                texture.unUse();
        };

        bool rotate = false;
        glm::vec3 rotateAxis = glm::vec3(0.0f, 1.0f, 0.0f); // what axis(es) to apply rotation to
        float rotationSpeed = 0.0f; // the speed at which we rotate
        glm::vec3 pivot = glm::vec3(0.0f); // what point to rotate around
        glm::vec3 rotation = glm::vec3(0.0f); // initial orientation

        float currentAngle = 0.0f; // to track rotation over time
        void update(float deltaTime) { // deltaTime is how long since last frame and current (i think)
            if (rotate) {
                currentAngle += rotationSpeed * deltaTime;
                if (currentAngle > glm::two_pi<float>()) currentAngle -= glm::two_pi<float>();
            }
        }
        
        glm::mat4 getMatrix() const {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            if (rotate) {
                model = glm::translate(model, pivot);
                model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0));
                model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
                model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
                model = glm::rotate(model, currentAngle, rotateAxis);
                model = glm::translate(model, -pivot);
            }
            return model;
        }

        bool getUseTexture() const {
            return useTexture;
        }

        ~Element() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
};

int main() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    // init shit
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(512,512, "Title", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initalize GLAD\n";
        return -1;
    }

    glViewport(0, 0, 512, 512);
    // callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    std::vector<Element*> Objects;
    Element quad;
    quad.vertices = {
//      X      Y      Z       R     G     B      TEXCOORDS     NX     NY    NZ
        -5.0f, 0.0f, -5.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f,// 0
        5.0f, 0.0f, -5.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,// 1
        5.0f, 0.0f, 5.0f,    1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,// 2
        -5.0f, 0.0f, 5.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,// 3
    };
    quad.indices = {
        0, 1, 2,
        2, 3, 0
    };
    quad.x = 0.0f;
    quad.y = -1.0f;
    quad.useTexture = true;
    quad.textureFile = "textures/doomerfesh.png";
    quad.init();
    Objects.push_back(&quad);
    
    Element cube;
    cube.vertices = {
        // Back face (Z = -0.5)
        -0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,   1.0f, 1.0f,    0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f,    0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f,    0.0f, 0.0f, -1.0f,
    
        // Front face (Z = +0.5)
        -0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,     0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,     0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,     0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,     0.0f,  0.0f,  1.0f,
    
        // Left face (X = -0.5)
        -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,     -1.0f, 0.0f, 0.0f,
    
        // Right face (X = +0.5)
         0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 1.0f,   1.0f, 1.0f,      1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 1.0f,   1.0f, 0.0f,      1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 0.0f, 1.0f,   0.0f, 1.0f,      1.0f, 0.0f, 0.0f,
    
        // Bottom face (Y = -0.5)
        -0.5f, -0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,      0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,      0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,      0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,      0.0f, -1.0f, 0.0f,
    
        // Top face (Y = +0.5)
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 1.0f,      0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,      0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,      0.0f, 1.0f, 0.0f,
    };
    cube.indices = {
        0,  1,  2,   2,  3,  0,   // back
        4,  5,  6,   6,  7,  4,   // front
        8,  9,  10,  10, 11, 8,   // left
        12, 13, 14,  14, 15, 12,   // right
        16, 17, 18,  18, 19, 16,   // bottom
        20, 21, 22,  22, 23, 20    // top
    };
    cube.x = 2.0f;
    cube.y = -1.0f;
    cube.useTexture = true;
    cube.rotate = true;
    cube.rotateAxis = glm::vec3(1.0f, 1.0f, 2.0f);
    cube.rotationSpeed = 1.0f;
    cube.pivot = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.textureFile = "textures/doomerfesh.png";

    cube.init();
    Objects.push_back(&cube);
    
    Element wall;
    wall.vertices = {
//      X      Y      Z       R     G     B      TEXCOORDS     NX     NY    NZ
        -5.0f, 0.0f, -5.0f,  1.0f, 1.0f, 1.0f,   0.0f, 0.0f,   0.0f, 1.0f, 0.0f,// 0
        5.0f, 0.0f, -5.0f,   1.0f, 1.0f, 1.0f,   1.0f, 0.0f,   0.0f, 1.0f, 0.0f,// 1
        5.0f, 0.0f, 5.0f,    1.0f, 1.0f, 1.0f,   1.0f, 1.0f,   0.0f, 1.0f, 0.0f,// 2
        -5.0f, 0.0f, 5.0f,   1.0f, 1.0f, 1.0f,   0.0f, 1.0f,   0.0f, 1.0f, 0.0f,// 3
    };
    wall.indices = {
        0, 1, 2,
        2, 3, 0
    };
    wall.x = 0.0f;
    wall.y = 0.0f;
    wall.z = -5.0f;
    wall.rotate = true;
    wall.rotation = glm::vec3(glm::radians(90.0f), glm::radians(0.0f), glm::radians(90.0f));
    wall.rotateAxis = glm::vec3(1.0f,1.0f,1.0f);
    wall.rotationSpeed = 1.0f;
    wall.useTexture = true;
    wall.textureFile = "textures/doomerfesh.png";
    wall.init();
    Objects.push_back(&wall);

    Element lightSource;
    lightSource.vertices = {
        // Back face (Z = -0.5)
        -0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,   0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
         0.5f, -0.5f, -0.5f,    1.0f, 1.0f, 1.0f,   0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
         0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,   0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
        -0.5f,  0.5f, -0.5f,    1.0f, 1.0f, 1.0f,   0.0f, 0.0f,    0.0f, 0.0f, -1.0f,
    
        // Front face (Z = +0.5)
        -0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,     0.0f,  0.0f,  1.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,     0.0f,  0.0f,  1.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,     0.0f,  0.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,     0.0f,  0.0f,  1.0f,
    
        // Left face (X = -0.5)
        -0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,     -1.0f, 0.0f, 0.0f,
    
        // Right face (X = +0.5)
         0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      1.0f, 0.0f, 0.0f,
    
        // Bottom face (Y = -0.5)
        -0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, -1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, -1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, -1.0f, 0.0f,
    
        // Top face (Y = +0.5)
        -0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   1.0f, 1.0f, 1.0f,   0.0f, 0.0f,      0.0f, 1.0f, 0.0f,
    };
    lightSource.indices = {
        0,  1,  2,   2,  3,  0,   // back
        4,  5,  6,   6,  7,  4,   // front
        8,  9,  10,  10, 11, 8,   // left
        12, 13, 14,  14, 15, 12,   // right
        16, 17, 18,  18, 19, 16,   // bottom
        20, 21, 22,  22, 23, 20    // top
    };
    lightSource.x = 5.0f;
    lightSource.y = 1.0f;
    lightSource.z = 0.0f;
    lightSource.init();
    Objects.push_back(&lightSource);

    // x,y,z
    Element indicator;
    indicator.vertices = {
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // 0
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,  0.0f, 0.0f, // 1

        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f, // 2
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f,  0.0f, 0.0f, // 3

        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // 4
        0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f,  0.0f, 0.0f, // 5
    };
    indicator.indices = {
        0, 1,
        2, 3,
        4, 5
    };
    indicator.draw_mode = GL_LINES;
    indicator.init();
    // Objects.push_back(&indicator);
    Shader objectShader("shaders/object.vert", "shaders/object.frag");
    Shader lightShader("shaders/light.vert", "shaders/light.frag");

    wall.shader = &objectShader;
    lightSource.shader = &lightShader;
    cube.shader = &objectShader;
    quad.shader = &objectShader;

    lightSource.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);

        glm::mat4 view = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraPos+cameraFront, cameraUp);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(75.0f), windowWidth / windowHeight, 0.1f, 100.0f);
        
        for (Element* e : Objects) {
            e->update(deltaTime);
            e->draw(view, projection, lightSource, cameraPos);
        };

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}