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
#include "player.hpp"
#include "shader_def.hpp"

float windowWidth = 512.0f;
float windowHeight = 512.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
    windowWidth = (float)width;
    windowHeight = (float)height;
} // called each time window is resized, and sets viewport size

float lastMouseX = windowWidth/2, lastMouseY = windowHeight/2;
Camera mainCamera;
Player defaultPlayer; // doing this for support for multiple players in the future
Player* controlledPlayer = &defaultPlayer;
bool inMenu = false;
float currentFrame = 0.0f, deltaTime = 0.0f, lastFrame = 0.0f;

// handle mouse movement, change pitch/yaw of mainCamera to match that of the mouse x and y offset
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (!controlledPlayer || !controlledPlayer->camera()) {
        std::cerr << "Player or camera is null!" << std::endl;
        return;
    }
    float xoffset = xpos - lastMouseX;
    float yoffset = lastMouseY - ypos;
    lastMouseX = xpos;
    lastMouseY = ypos;

    const float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    controlledPlayer->camera()->setYaw(controlledPlayer->camera()->getYaw()+xoffset);
    controlledPlayer->camera()->setPitch(controlledPlayer->camera()->getPitch()+yoffset);
}

void processInput(GLFWwindow *window, Player* player) {
    if (!controlledPlayer) {
        std::cerr << "Player is null! 58" << std::endl;
        return;
    }
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (!inMenu) {
        player->keyInput(window, deltaTime);
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
    initShaders();
    std::vector<Element*> Objects; // create Objects list

    controlledPlayer->init(Objects, &mainCamera);
    controlledPlayer->setPosition(glm::vec3(0.0f,10.0f,0.0f));
    if (controlledPlayer->camera()) {
        printf("%f\n", controlledPlayer->camera()->getOrientation().x);
    } else {
        printf("Camera pointer is null!\n");
    }

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
    cube.bounding_box_corner2 = glm::vec3(0.5f, 0.5f,  0.5f);
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
    // cube.anchored = true;
    cube.gravity = false;

    cube.init();
    addToWorld(&cube, Objects);

    Element wall1;
    wall1.vertices = {QUAD_VERTICES};
    wall1.indices = {QUAD_INDICES};
    wall1.bounding_box_corner1 = glm::vec3(-2.0f, -0.1f, -2.0f);
    wall1.bounding_box_corner2 = glm::vec3(2.0f, 0.0f, 2.0f);
    wall1.position = glm::vec3(2.0f, 2.0f, 4.0f);
    wall1.sizex = 2.0f;
    wall1.sizey = 2.0f;
    wall1.sizez = 2.0f;
    wall1.rotate = true;
    wall1.rotation = glm::vec3(1.0f,0.0f,0.0f);
    wall1.anchored = true;
    wall1.init();
    addToWorld(&wall1,Objects);

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


    lightSource.lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    // apply shaders
    for (size_t i = 0; i<Objects.size();i++) {
        if (Objects[i]->debug)
            Objects[i]->shader = debugShader;
        else
            Objects[i]->shader = objectShader;
    }
    lightSource.shader = lightShader;

    float dt = 1.0f/60.0f;
    float accumulator = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(window)) {
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        controlledPlayer->update();
        processInput(window, controlledPlayer);
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

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(75.0f), windowWidth / windowHeight, 0.1f, 100.0f);
        // printf("player pos: %f %f %f\n", controlledPlayer->getPosition().x, controlledPlayer->getPosition().y,controlledPlayer->getPosition().z);
        // printf("controlledplayer->camera()->front.x: %f\n", controlledPlayer->camera()->getOrientation().x);
        for (Element* e : Objects) {
            e->update(deltaTime, Objects);
            e->draw(controlledPlayer->camera()->view(), projection, lightSource, controlledPlayer->camera()->getPos(), glfwGetTime());
        };
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}