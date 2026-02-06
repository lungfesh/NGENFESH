#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "util.hpp"
// #include "lighting.hpp"


float windowWidth = 512.0f;
float windowHeight = 512.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = (float)width;
    windowHeight = (float)height;
} // called each time window is resized, and sets viewport size

float lastMouseX = windowWidth/2, lastMouseY = windowHeight/2;


// implemented fps style movement, cannot get freecam to work again
bool fpsMovement = true;

Camera mainCamera;

// handle mouse movement, change pitch/yaw of mainCamera to match that of the mouse x and y offset
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

class Element {
    public:
        unsigned int VAO = 0, VBO = 0, EBO = 0;
        glm::vec3 position{0.0f};
        glm::vec3 lastPosition{0.0f};
        glm::vec3 velocity{0.0f};
        bool wireframe = false;
        GLenum draw_mode = GL_TRIANGLES;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::string textureFile = "";
        bool useTexture = false;
        Texture texture;

        Shader* shader = nullptr;
        glm::vec3 lightColor;

        glm::vec3 bounding_box_corner1{-0.5}; // for a 1x1 unit cube
        glm::vec3 bounding_box_corner2{0.5};
        bool affectedByGravity = false;
        bool grounded = false;

        Element* debugElement = nullptr;
        bool debug = false;
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
        void update(float deltaTime, std::vector<Element*>& Objects) { // deltaTime is how long since last frame and current (i think)
            if (rotate) {
                currentAngle += rotationSpeed * deltaTime;
                if (currentAngle > glm::two_pi<float>()) currentAngle -= glm::two_pi<float>();
            }
            if (lastPosition != position) {
                for (size_t i = 0; i < Objects.size(); i++) {
                    if (Objects[i] == this) {continue;}
                    if (Objects[i]->debug == true) {continue;}
                    if (Objects[i]->position == position) {continue;} // oh the horrors
                    if (AABBCollideDetect(position+bounding_box_corner1,
                    position+bounding_box_corner2,
                    Objects[i]->position+Objects[i]->bounding_box_corner1, 
                    Objects[i]->position+Objects[i]->bounding_box_corner2)) {
                        // printf("Collision at %f, pos1: %f %f %f, pos2: %f %f %f\n", glfwGetTime(),Objects[i]->position.x, Objects[i]->position.y, Objects[i]->position.z, position.x, position.y, position.z);
                        float px = std::min(position.x+bounding_box_corner2.x, Objects[i]->position.x+Objects[i]->bounding_box_corner2.x) - std::max(position.x+bounding_box_corner1.x, Objects[i]->position.x+Objects[i]->bounding_box_corner1.x);
                        float py = std::min(position.y+bounding_box_corner2.y, Objects[i]->position.y+Objects[i]->bounding_box_corner2.y) - std::max(position.y+bounding_box_corner1.y, Objects[i]->position.y+Objects[i]->bounding_box_corner1.y);
                        float pz = std::min(position.z+bounding_box_corner2.z, Objects[i]->position.z+Objects[i]->bounding_box_corner2.z) - std::max(position.z+bounding_box_corner1.z, Objects[i]->position.z+Objects[i]->bounding_box_corner1.z);
                        // thanks chatgpt
                        // we're checking which axis has the most overlap, setting pos of 1 object to not be inside the other, setting vel on that axis to 0
                        // this should probably have it's own func
                        if (px < py && px < pz) {
                            float dir = (position.x < Objects[i]->position.x) ? -1.0f : 1.0f;
                            position.x += px * dir;
                            velocity.x = 0;
                        }
                        else if (py < pz) {
                            float dir = (position.y < Objects[i]->position.y) ? -1.0f : 1.0f;
                            position.y += py * dir;
                            velocity.y = 0;
                        }
                        else {
                            float dir = (position.z < Objects[i]->position.z) ? -1.0f : 1.0f;
                            position.z += pz * dir;
                            velocity.z = 0;
                        }
                    }
                }
            }
            grounded = (velocity.y == 0.0f ? true : false);
            if (debugElement != nullptr) debugElement->position = position;
            lastPosition = position;
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
            if (affectedByGravity) {
                velocity.y += -9.8*dt;    
            }

            // now dampen so it doesn't fly forever

            float damping = 2.0f; // units per second
            if (glm::length(velocity) > 0.0f) {
                glm::vec3 decel = glm::normalize(velocity) * damping * dt;
                if (glm::length(decel) > glm::length(velocity))
                    velocity = glm::vec3(0.0f); // stop completely
                else {
                    if (affectedByGravity) velocity -= decel;
                    else {
                        velocity.x -= decel.x; // let gravity handle vel.y
                        velocity.z -= decel.z;
                    }
                }
            }
            
            // some of the shit in Element::update should probably go in here

            position += velocity * dt; // apply velocity
        }

        ~Element() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
};

class HUDElement {
    public:
        unsigned int VAO = 0, VBO = 0, EBO = 0;
        glm::vec3 position{0.0f};
        bool wireframe = false;
        GLenum draw_mode = GL_TRIANGLES;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::string textureFile = "";
        bool useTexture = false;
        Texture texture;

        Shader* shader = nullptr;
        glm::vec3 lightColor;

        HUDElement* debugElement = nullptr;
        bool debug = false;
        HUDElement() = default;

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
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
            glEnableVertexAttribArray(1);
        
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
            glEnableVertexAttribArray(2);
        };
        void draw() const {
            if (!shader) return;


            shader->use();
            shader->setBool("useTexture", getUseTexture());
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
        glm::vec2 rotateAxis = glm::vec2(0.0f, 1.0f); // what axis(es) to apply rotation to
        float rotationSpeed = 0.0f; // the speed at which we rotate
        glm::vec2 pivot = glm::vec2(0.0f); // what point to rotate around
        glm::vec2 rotation = glm::vec2(0.0f); // initial orientation

        float currentAngle = 0.0f; // to track rotation over time
        void update(float deltaTime) { // deltaTime is how long since last frame and current (i think)
            if (rotate) {
                currentAngle += rotationSpeed * deltaTime;
                if (currentAngle > glm::two_pi<float>()) currentAngle -= glm::two_pi<float>();
            }
        }

        bool getUseTexture() const {
            return useTexture;
        }

        ~HUDElement() {
            glDeleteVertexArrays(1, &VAO);
            glDeleteBuffers(1, &VBO);
            glDeleteBuffers(1, &EBO);
        }
};

void processInput(GLFWwindow *window, Element* player) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float cameraSpeed = mainCamera.speed * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(mainCamera.front,mainCamera.up));
    if (fpsMovement) {
        glm::vec3 forward = mainCamera.front;
        forward.y = 0.0f;
        if (glm::length(forward) > 0.0f)
            forward = glm::normalize(forward);


        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            player->velocity += cameraSpeed * forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            player->velocity -= cameraSpeed * forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player->velocity -= right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player->velocity += right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            if (!player->grounded) {return;}
            player->velocity += 10.0f * mainCamera.up;
        }
    }
    else {
        glm::vec3 forward = mainCamera.front;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            player->velocity += cameraSpeed * forward;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            player->velocity -= cameraSpeed * forward;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            player->velocity -= right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            player->velocity += right * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
            player->velocity += cameraSpeed * mainCamera.up;
        }
        }
}

int main() {
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    // init shit
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(512,512, "NGENFESH", NULL, NULL);
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
    quad.bounding_box_corner1 = glm::vec3(-5.0f,0.0f,-5.0f);
    quad.bounding_box_corner2 = glm::vec3(5.0f,0.1f,5.0f);
    quad.position.x = 0.0f;
    quad.position.y = -1.0f;
    quad.useTexture = true;
    quad.textureFile = "textures/doomerfesh.png";
    quad.init();
    Objects.push_back(&quad);
    
    Element quadBB;
    quadBB.vertices = calcBoundingBoxVerts(quad.bounding_box_corner1, quad.bounding_box_corner2);
    quadBB.indices = {
    0,1,  1,3,  3,2,  2,0, // front face edges
    4,5,  5,7,  7,6,  6,4, // back face edges
    0,4,  1,5,  2,6,  3,7  // connecting edges
    };
    quadBB.position = quad.position;
    quadBB.draw_mode = GL_LINES;
    quadBB.debug = true;
    quadBB.init();
    Objects.push_back(&quadBB);
    quad.debugElement = &quadBB;

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
        -0.5f,  0.5f, -0.5f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,      0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,      0.0f, 1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,      0.0f, 1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,   0.0f, 1.0f, 0.0f,   0.0f, 1.0f,      0.0f, 1.0f, 0.0f,
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
    cube.position.y = 2.0f;
    cube.useTexture = true;
    cube.rotate = true;
    cube.rotateAxis = glm::vec3(1.0f, 1.0f, 2.0f);
    cube.rotationSpeed = 5.0f;
    cube.pivot = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.textureFile = "textures/doomerfesh.png";

    cube.init();
    Objects.push_back(&cube);
    
    Element cubeBB;
    cubeBB.vertices = calcBoundingBoxVerts(cube.bounding_box_corner1, cube.bounding_box_corner2);
    cubeBB.indices = {
    0,1,  1,3,  3,2,  2,0, // front face edges
    4,5,  5,7,  7,6,  6,4, // back face edges
    0,4,  1,5,  2,6,  3,7  // connecting edges
    };
    cubeBB.position = cube.position;
    cubeBB.draw_mode = GL_LINES;
    cubeBB.debug = true;
    cubeBB.init();
    Objects.push_back(&cubeBB);
    cube.debugElement = &cubeBB;

    Element player;
    // player.vertices = cube.vertices;
    // player.indices = cube.indices;
    player.position = mainCamera.pos;
    player.useTexture = true;
    player.textureFile = "textures/doomerfesh.png";
    player.init();
    Objects.push_back(&player);
    player.affectedByGravity = true;

    Element lightSource;
    lightSource.vertices = cube.vertices;
    lightSource.indices = cube.indices;
    lightSource.position.x = 5.0f;
    lightSource.position.y = 1.0f;
    lightSource.position.z = 0.0f;
    lightSource.init();
    Objects.push_back(&lightSource);

    Shader objectShader("shaders/object.vert", "shaders/object.frag");
    Shader lightShader("shaders/light.vert", "shaders/light.frag");
    Shader debugShader("shaders/object.vert", "shaders/solid.frag");

    lightSource.shader = &lightShader;
    player.shader = &objectShader;
    cube.shader = &objectShader;
    quad.shader = &objectShader;
    cubeBB.shader = &debugShader;
    quadBB.shader = &debugShader;
    lightSource.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    glm::vec3 cameraOffset = glm::vec3(0.0f,1.0f,0.0f);

    std::vector<HUDElement*> HUDObjects;
    HUDElement square;
    square.vertices = {
       -1.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        0.5f,  1.0f, 0.0f,  0.0f, 1.0f, 0.0f,  0.0f, 0.0f,
       -1.0f,  0.0f, 0.0f,  0.0f, 0.0f, 1.0f,  0.0f, 0.0f,
        0.5f,  0.0f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0f, 0.0f
    };
    square.indices = {
        0,1,2, 2,1,3
    };
    square.position.x = 0.0f;
    square.position.y = 0.0f;
    square.init();
    // HUDObjects.push_back(&square);

    Shader hudShader("shaders/hud.vert", "shaders/hud.frag");
    square.shader = &hudShader;

    float dt = 1.0f/60.0f;
    float accumulator = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(window)) {
        processInput(window, &player);

        // physics shit
        // cube.velocity = mainCamera.pos - cube.position;
        
        mainCamera.pos = player.position + cameraOffset;
        accumulator += deltaTime;
        while (accumulator >= dt) {
            player.physics_step(dt);
            for (Element* e : Objects) {
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
            e->update(deltaTime, Objects);
            e->draw(mainCamera.view(), projection, lightSource, mainCamera.pos);
        };
        for (HUDElement* e : HUDObjects) {
            e->update(deltaTime);
            e->draw();
        };
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();

    return 0;
}