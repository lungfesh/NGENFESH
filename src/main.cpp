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
#include "element.hpp"
#include "premade_elements.hpp"

float windowWidth = 512.0f;
float windowHeight = 512.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = (float)width;
    windowHeight = (float)height;
} // called each time window is resized, and sets viewport size

float lastMouseX = windowWidth/2, lastMouseY = windowHeight/2;

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


void processInput(GLFWwindow *window, Element* player) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float cameraSpeed = mainCamera.speed * deltaTime;
    glm::vec3 right = glm::normalize(glm::cross(mainCamera.front,mainCamera.up));
    glm::vec3 forward = mainCamera.front;
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);

    glm::vec3 moveDir = glm::vec3(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // 0
        moveDir += forward;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveDir -= forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveDir -= right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveDir += right;
    player->velocity.x = moveDir.x * cameraSpeed;
    player->velocity.z = moveDir.z * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!player->grounded) {return;}
        player->velocity.y = 10.0f;
    }
}

GLFWwindow* initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(512,512, "NGENFESH", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initalize GLAD\n";
        return NULL;
    }

    glViewport(0, 0, 512, 512);
    // callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    return window;
}

int main() {
    // std::random_device rd;
    // std::mt19937 gen(rd());
    // std::uniform_real_distribution<float> dist(-1.0f, 1.0f);

    GLFWwindow* window = initWindow();
    if (window == NULL) {
        std::cerr << "Error creating window! Closing..";
        return -1;
    }
    std::vector<Element*> Objects; // create Objects list

    // start creating objects
    Element quad;
    quad.vertices = {QUAD_VERTICES};
    quad.indices = {QUAD_INDICES};

    quad.bounding_box_corner1 = glm::vec3(-5.0f,0.0f,-5.0f);
    quad.bounding_box_corner2 = glm::vec3(5.0f,0.1f,5.0f);
    quad.position.x = 0.0f;
    quad.position.y = -1.0f;
    quad.useTexture = true;
    quad.textureFile = "textures/doomerfesh.png";
    quad.anchored = true;
    quad.sizex = 5.0f;
    quad.sizey = 1.0f;
    quad.sizez = 5.0f;
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
    cube.vertices = {CUBE_VERTICES};
    cube.indices = {CUBE_INDICES};
    cube.position.x = 2.0f;
    cube.position.y = 2.0f;
    cube.useTexture = true;
    cube.rotate = true;
    cube.rotateAxis = glm::vec3(1.0f, 1.0f, 2.0f);
    cube.rotationSpeed = 5.0f;
    cube.pivot = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.textureFile = "textures/doomerfesh.png";
    cube.bounce = false;

    cube.init();
    Objects.push_back(&cube);
    
    Element cubeBB;
    cubeBB.vertices = calcBoundingBoxVerts(cube.bounding_box_corner1, cube.bounding_box_corner2);
    cubeBB.indices = {CUBEBB_INDICES};
    cubeBB.position = cube.position;
    cubeBB.draw_mode = GL_LINES;
    cubeBB.debug = true;
    cubeBB.init();
    Objects.push_back(&cubeBB);
    cube.debugElement = &cubeBB;

    Element player;
    player.bounding_box_corner1 = glm::vec3(-0.5);
    player.bounding_box_corner2 = glm::vec3(0.5f, 1.5f, 0.5f);
    player.vertices = calcBoundingBoxVerts(player.bounding_box_corner1, player.bounding_box_corner2, glm::vec3(1.0f));
    player.indices = {CUBEBB_INDICES};

    player.position = mainCamera.pos;
    player.useTexture = true;
    player.textureFile = "textures/doomerfesh.png";
    player.draw_mode = GL_LINES;
    player.debug = true;
    player.init();
    Objects.push_back(&player);
    player.anchored = false;
    player.hasCollision = true;
    player.isPlayer = true;

    Element lightSource;
    lightSource.vertices = cube.vertices;
    lightSource.indices = cube.indices;
    lightSource.position.x = 5.0f;
    lightSource.position.y = 1.0f;
    lightSource.position.z = 0.0f;
    lightSource.init();
    Objects.push_back(&lightSource);

    Element lightSource2 = Element(lightSource);
    lightSource2.position.z = 2.0f;
    lightSource2.init();
    Objects.push_back(&lightSource2);

    // create shaders
    Shader objectShader("shaders/object.vert", "shaders/object.frag");
    Shader lightShader("shaders/light.vert", "shaders/light.frag");
    Shader debugShader("shaders/object.vert", "shaders/solid.frag");

    lightSource.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lightSource2.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    // apply shaders
    for (size_t i = 0; i<Objects.size();i++) {
        if (Objects[i]->debug)
            Objects[i]->shader = &debugShader;
        else
            Objects[i]->shader = &objectShader;
    }

    lightSource.shader = &lightShader;
    lightSource2.shader = &lightShader;

    glm::vec3 cameraOffset = glm::vec3(0.0f,1.0f,0.0f);

    // Shader hudShader("shaders/hud.vert", "shaders/hud.frag");
    // square.shader = &hudShader;

    float dt = 1.0f/60.0f;
    float accumulator = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(window)) {
        processInput(window, &player);

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
            e->draw(mainCamera.view(), projection, lightSource, mainCamera.pos, glfwGetTime());
        };
        // for (HUDElement* e : HUDObjects) {
            // e->update(deltaTime);
            // e->draw();
        // };
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}