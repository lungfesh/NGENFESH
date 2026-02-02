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

struct Camera {
    glm::vec3 pos{0.0f};
    glm::vec3 front{0.0f};
    glm::vec3 up{0.0f};

    float yaw = 0.0f;
    float pitch = 0.0f;
    float speed = 2.5f;
    float sensitivity = 0.1f;

    glm::mat4 view() const {
        return glm::lookAt(pos,pos+front,up);
    }
};

Camera mainCamera {
    .pos = glm::vec3(0.0f, 0.0f, 3.0f),
    .front = glm::vec3(0.0f, 0.0f, -1.0f),
    .up = glm::vec3(0.0f, 1.0f, 0.0f)
};

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    mainCamera.yaw += xoffset;
    mainCamera.pitch += yoffset;

    if (mainCamera.pitch > 89.9f)
        mainCamera.pitch = 89.9f;
    if (mainCamera.pitch < -89.9f)
        mainCamera.pitch = -89.9f;
}
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
bool fpsMovement = false;

void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float cameraSpeed = mainCamera.speed * deltaTime;
    glm::vec3 forward;


    if (fpsMovement){
        forward.x = cos(glm::radians(mainCamera.yaw));
        forward.y = 0.0f;
        forward.z = sin(glm::radians(mainCamera.yaw));
        forward = glm::normalize(forward);
    } else {
        forward = mainCamera.front;
    }

    glm::vec3 right = glm::normalize(glm::cross(mainCamera.front,mainCamera.up));
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        mainCamera.pos += cameraSpeed * forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        mainCamera.pos -= cameraSpeed * forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        mainCamera.pos -= right * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        mainCamera.pos += right * cameraSpeed;
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
        glm::vec3 position;
        glm::vec3 velocity;
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
            shader->setVec3("lightPos", glm::vec3(lightSource.position.x, lightSource.position.y, lightSource.position.z));
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
            model = glm::translate(model, position);
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

        void physics_step(float dt) {
            // if (position.y >= -1.0f) {
                // velocity.y += -9.8*dt;    
            // }
            // velocity.y += -9.8*dt;

            position += velocity * dt; // apply velocity

            // now dampen so it doesn't fly forever
            float damping = 2.0f; // units per second
            if (glm::length(velocity) > 0.0f) {
                glm::vec3 decel = glm::normalize(velocity) * damping * dt;
                if (glm::length(decel) > glm::length(velocity))
                    velocity = glm::vec3(0.0f); // stop completely
                else
                    velocity -= decel;
            }
        }

        ~Element() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
};

// given position, go thru Element list and check if any Element is within 1u of position.
// if true, calc dir from Element.pos + 1u to given position, then apply force to Element in the -direction
void detectCollision(glm::vec3 position, std::vector<Element*> Objects) {
    // yes, yes, yes, i know. it's called detect collisions but it applies some velocities and shit aswell. calm your tits
    for (Element* e : Objects) {
        if (position == e->position) continue; // skip over itself
        glm::vec3 diff = position-e->position;
        if (glm::length(diff) <= 1.0f) {
            glm::vec3 direction = glm::normalize(diff);

            e->velocity -= direction*2.0f;
        }
    } // holy terrible code
}

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
    quad.position.x = 0.0f;
    quad.position.y = -1.0f;
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
    cube.position.x = 2.0f;
    cube.position.y = -1.0f;
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
    wall.position.x = 0.0f;
    wall.position.y = 0.0f;
    wall.position.z = -5.0f;
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
    lightSource.position.x = 5.0f;
    lightSource.position.y = 1.0f;
    lightSource.position.z = 0.0f;
    lightSource.init();
    Objects.push_back(&lightSource);

    Shader objectShader("shaders/object.vert", "shaders/object.frag");
    Shader lightShader("shaders/light.vert", "shaders/light.frag");

    wall.shader = &objectShader;
    lightSource.shader = &lightShader;
    cube.shader = &objectShader;
    quad.shader = &objectShader;

    lightSource.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    float dt = 1.0f/60.0f;
    float accumulator = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // physics shit
        accumulator += deltaTime;
        while (accumulator >= dt) {
            for (Element* e : Objects) {
                detectCollision(mainCamera.pos, Objects);
                detectCollision(e->position, Objects);
                e->physics_step(dt);
            }
            accumulator -= dt;    
        } 

        // now onto rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

        glm::vec3 direction;
        direction.x = cos(glm::radians(mainCamera.yaw)) * cos(glm::radians(mainCamera.pitch));
        direction.y = sin(glm::radians(mainCamera.pitch));
        direction.z = sin(glm::radians(mainCamera.yaw)) * cos(glm::radians(mainCamera.pitch));
        mainCamera.front = glm::normalize(direction);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(75.0f), windowWidth / windowHeight, 0.1f, 100.0f);

        for (Element* e : Objects) {
            e->update(deltaTime);
            e->draw(mainCamera.view(), projection, lightSource, mainCamera.pos);
        };

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}