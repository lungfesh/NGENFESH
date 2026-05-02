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

KeyState keys[512] = {};
int keysToCheck[512] = {
    GLFW_KEY_W,
    GLFW_KEY_A,
    GLFW_KEY_S,
    GLFW_KEY_D,
    GLFW_KEY_E,
    GLFW_KEY_F,
    GLFW_KEY_SPACE,
    GLFW_KEY_ESCAPE
}; // if this gets bigger, more complex, user defined keys, etc, more complex input system should be made
//                                                               including callbacks, etc

void processInput(GLFWwindow* window) {
    // if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){ /*glfwSetWindowShouldClose(window, true);*/

    // }
    for (int key : keysToCheck) {
        int keyState = glfwGetKey(window, key);
        if (keyState == GLFW_PRESS) {
            keys[key].pastState = keys[key].currentState;
            keys[key].currentState = true;
            // if (keys[key].currentState != keys[key].pastState)
                // printf("%c PRESSED    CUR: %i PREV: %i\n", key, keys[key].currentState, keys[key].pastState);
        }
        if (keyState == GLFW_RELEASE) {
            keys[key].pastState = keys[key].currentState;
            keys[key].currentState = false;
            // if (keys[key].currentState != keys[key].pastState)
                // printf("%c IS RELEASED       CUR: %i PREV: %i\n", key, keys[key].currentState, keys[key].pastState);
        }
    }
    controlledPlayer->keyInput(deltaTime, keys, window);
}

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
    // glfwSetKeyCallback(window, key_callback);
    if (window == NULL) {
        std::cerr << "Error creating window! Closing..";
        return -1;
    }
    initShaders();
    std::vector<Element*> Objects; // create Objects list

    controlledPlayer->setWorld(&Objects);
    controlledPlayer->init(Objects, &mainCamera);
    controlledPlayer->setPosition(glm::vec3(0.0f,10.0f,0.0f));
    if (!controlledPlayer->camera()) {
        printf("Camera pointer is null!\n");
    }
    // Element playerBB;
    // playerBB.debug = true;
    // playerBB.vertices = calcBoundingBoxVerts(controlledPlayer->playerElement.bounding_box_corner1, controlledPlayer->playerElement.bounding_box_corner2, glm::vec3{1.0f}, true);
    // playerBB.indices = {CUBEBB_INDICES};
    // playerBB.position = glm::vec3(2.0f, 2.0f, 5.0f);
    // playerBB.sizex = 1.0f;
    // playerBB.sizey = 1.0f;
    // playerBB.sizez = 1.0f;
    // // wall1.rotate = true;
    // playerBB.rotation = glm::vec3(1.0f,0.0f,0.0f);
    // playerBB.anchored = true;
    // playerBB.hasCollision = false;
    // // cubeBB.draw_mode = GL_LINES;
    // playerBB.debug = true;
    // playerBB.init();
    // addToWorld(&playerBB,Objects);
    // controlledPlayer->playerElement.debugElement = &playerBB;

    // start creating objects
    Element quad;
    quad.vertices = {QUAD_VERTICES};
    quad.indices = {QUAD_INDICES};

    quad.bounding_box_corner1 = glm::vec3(-20.0f,-0.1f,-20.0f);
    quad.bounding_box_corner2 = glm::vec3(20.0f,0.0f,20.0f);
    quad.position.x = 0.0f;
    quad.position.y = -1.0f;
    quad.useTexture = true;
    quad.textureFile = "textures/grass.jpg";
    quad.anchored = true;
    quad.sizex = 20.0f;
    quad.sizey = 1.0f;
    quad.sizez = 20.0f;
    quad.holdable = false;
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
    std::vector<glm::vec3> cubePoints = calcBoundingBoxPoints(cube.vertices);
    cube.bounding_box_corner1 = cubePoints[0];
    cube.bounding_box_corner2 = cubePoints[1];
    cube.position.x = 5.0f;
    cube.position.y = 2.0f;
    cube.useTexture = true;
    cube.rotate = true;
    // cube.rotateAxis = glm::vec3(1.0f, 1.0f, 2.0f);
    // cube.rotationSpeed = 5.0f;
    cube.pivot = glm::vec3(0.0f, 0.0f, 0.0f);
    cube.textureFile = "textures/lungfesh.png";
    for (size_t i = 0; i < cube.vertices.size(); i=i+11) {
        cube.vertices[i+3] = 1.0f;
        cube.vertices[i+4] = 1.0f;
        cube.vertices[i+5] = 1.0f;
    }
    cube.init();
    addToWorld(&cube, Objects);
    Element wall1;
    wall1.vertices = {QUAD_VERTICES};
    for (size_t i = 0; i < wall1.vertices.size(); i=i+11) { // change wall1 colour to red, should probably make a function for this
        wall1.vertices[i+3] = 1.0f;
        wall1.vertices[i+4] = 0.0f;
        wall1.vertices[i+5] = 0.0f;
    }
    wall1.indices = {QUAD_INDICES};
    std::vector<glm::vec3> wall1Points = calcBoundingBoxPoints(wall1.vertices);
    wall1.bounding_box_corner1 = wall1Points[0] - .1f;
    wall1.bounding_box_corner2 = wall1Points[1];
    wall1.position = glm::vec3(2.0f, 2.0f, 4.0f);
    wall1.init();
    addToWorld(&wall1,Objects);
  
    Element lightSource;
    lightSource.vertices = {CUBE_VERTICES};
    lightSource.indices = {CUBE_INDICES};
    lightSource.position.x = 5.0f;
    lightSource.position.y = 3.0f;
    lightSource.position.z = 0.0f;

    lightSource.emitPointLight = true;
    lightSource.pointLightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    lightSource.init();
    addToWorld(&lightSource, Objects);

    // apply shaders
    for (size_t i = 0; i<Objects.size();i++) {
        if (Objects[i]->debug)
            Objects[i]->shader = debugShader;
        else
            Objects[i]->shader = objectShader;
    }
    // lightSource.shader = lightShader;

    float dt = 1.0f/60.0f;
    float accumulator = 0.0f;

    glEnable(GL_DEPTH_TEST);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    printf("size of pointlights: %i\n", PointLights.size());
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        controlledPlayer->update();
        accumulator += deltaTime;
        while (accumulator >= dt) {
            for (Element* e : Objects) {
                e->physics_step(dt,Objects);
            }
            accumulator -= dt;    
        } 
        // now onto rendering
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glClearColor(0.0f,0.0f,0.0f,1.0f);

        glm::mat4 projection;
        projection = glm::perspective(glm::radians(75.0f), windowWidth / windowHeight, 0.1f, 100.0f);
        for (size_t i = 0; i < PointLights.size(); i++) {
            printf("iteration %i-----\n", i);
            printf("POSITION %f, %f, %f\n", PointLights[i]->position.x, PointLights[i]->position.y, PointLights[i]->position.z);
            printf("COLOR %f, %f, %f\n", PointLights[i]->pointLightColor.x, PointLights[i]->pointLightColor.y, PointLights[i]->pointLightColor.z);
            printf("SPEC STRENGTH %f\n", PointLights[i]->pointLightSpecStrength);
            objectShader->setVec3("pointLights[" + std::to_string(i) + "].position", PointLights[i]->position);
            objectShader->setVec3("pointLights[" + std::to_string(i) + "].color", PointLights[i]->pointLightColor);
            objectShader->setFloat("pointLights[" + std::to_string(i) + "].specularStrength", PointLights[i]->pointLightSpecStrength);
        }
        objectShader->setInt("numPointLights", PointLights.size());
        for (Element* e : Objects) {
            e->update(deltaTime);
            e->draw(controlledPlayer->camera()->view(), projection, lightSource, controlledPlayer->camera()->getPos(), glfwGetTime());
        };
        glfwSwapBuffers(window);
        glfwPollEvents();
        printf("done rendering frame -----\n");
    }
    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}