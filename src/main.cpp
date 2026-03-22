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
        if (player->grounded == false) {return;}
        player->velocity.y = 10.0f;
    }
    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
        // printf("breakeeeee\n");
        printf("id: %i, pressed g, grounded is set to %s\n", player->id, (player->grounded) ? "true" : "false");
        // player->velocity = glm::vec3{0.0f};
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

    quad.bounding_box_corner1 = glm::vec3(-20.0f,0.0f,-20.0f);
    quad.bounding_box_corner2 = glm::vec3(20.0f,0.1f,20.0f);
    quad.position.x = 0.0f;
    quad.position.y = -1.0f;
    quad.useTexture = true;
    quad.textureFile = "textures/grass.jpg";
    quad.anchored = true;
    quad.sizex = 20.0f;
    quad.sizey = 1.0f;
    quad.sizez = 20.0f;
    quad.init();
    addToWorld(&quad, Objects);

    Element skybox1;
    skybox1.vertices = {QUAD_VERTICES};
    skybox1.indices = {QUAD_INDICES};
    skybox1.useTexture = true;
    skybox1.textureFile = "textures/sky.jpeg";
    skybox1.hasCollision = false;
    skybox1.anchored = true;
    skybox1.sizex = 20.0f;
    skybox1.sizey = 1.0f;
    skybox1.sizez = 20.0f;
    skybox1.position.x = 0.0f;
    skybox1.position.y = 20.0f;
    skybox1.init();
    addToWorld(&skybox1, Objects);

    Element cube;
    cube.vertices = {CUBE_VERTICES};
    cube.indices = {CUBE_INDICES};
    cube.bounding_box_corner1 = glm::vec3(-0.5f, -0.5f, -0.5f);
    cube.bounding_box_corner1 = glm::vec3(0.5f, 0.5f, 0.5f);
    cube.position.x = 5.0f;
    cube.position.y = 5.0f;
    cube.useTexture = true;
    cube.rotate = true;
    cube.rotateAxis = glm::vec3(1.0f, 1.0f, 2.0f);
    cube.rotationSpeed = 5.0f;
    cube.pivot = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.textureFile = "textures/doomerfesh.png";
    // cube.bounce = true;
    // cube.bounce_amount = .75f;
    cube.anchored = true;

    cube.init();
    addToWorld(&cube, Objects);

    Element player;
    player.wireframe = true;
    // player.vertices = {CUBE_VERTICES};
    player.position = glm::vec3(0.0f,5.0f,0.0f);
    player.bounding_box_corner1 = glm::vec3(-0.5);
    player.bounding_box_corner2 = glm::vec3(0.5f, 1.5f, 0.5f);
    player.vertices = calcBoundingBoxVerts(player.bounding_box_corner1, player.bounding_box_corner2, glm::vec3(1.0f,0.0f,0.0f));
    // player.vertices = {CUBE_VERTICES};
    player.indices = {    0, 1, 2,
    2, 3, 0,

    // Top face
    4, 5, 6,
    6, 7, 4,

    // Front face (minZ)
    0, 1, 5,
    5, 4, 0,

    // Back face (maxZ)
    3, 2, 6,
    6, 7, 3,

    // Left face (minX)
    0, 3, 7,
    7, 4, 0,

    // Right face (maxX)
    1, 2, 6,
    6, 5, 1};
    player.anchored = false;
    player.hasCollision = true;
    player.isPlayer = true;
    player.init();
    addToWorld(&player, Objects);

    Element lightSource;
    lightSource.vertices = cube.vertices;
    lightSource.indices = cube.indices;
    lightSource.position.x = 5.0f;
    lightSource.position.y = 1.0f;
    lightSource.position.z = 0.0f;
    lightSource.init();
    addToWorld(&lightSource, Objects);

    // for (Element* e : Objects) {
    //     printf("init object id of %i pos %f %f %f %s\n", e->id, e->position.x, e->position.y, e->position.z, (e->isPlayer) ? "Is player" : "");
    // }

    // create shaders
    Shader objectShader("shaders/object.vert", "shaders/object.frag");
    Shader lightShader("shaders/light.vert", "shaders/light.frag");
    Shader debugShader("shaders/debug.vert", "shaders/debug.frag");

    lightSource.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    // apply shaders
    for (size_t i = 0; i<Objects.size();i++) {
        if (Objects[i]->debug)
            Objects[i]->shader = &debugShader;
        else
            Objects[i]->shader = &objectShader;
    }
    lightSource.shader = &lightShader;

    glm::vec3 cameraOffset = glm::vec3(0.0f,1.0f,0.0f);

    float dt = 1.0f/60.0f;
    float accumulator = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(window)) {
        processInput(window, &player);

        mainCamera.pos = player.position + cameraOffset;
        accumulator += deltaTime;
        while (accumulator >= dt) {
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
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}