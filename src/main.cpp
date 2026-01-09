#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

const char *vertexShaderSource =
"#version 460 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"out vec3 color;\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main() {\n"
    "gl_Position = projection * view * model * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "color = aColor;\n"
"}\0";

const char *fragmentShaderSource = 
"#version 460 core\n"
"out vec4 FragColor;\n"
"in vec3 color;\n"
"void main() {\n"
    "FragColor = vec4(color, 1.0f);\n"
"}\0";

glm::vec3 cameraPos = glm::vec3(0.0f,0.0f,3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f,0.0f,-1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // if (glfwGetMouseButton(window, 0) && !lastLeftClickState) {
    //     if (rotateUp) {
    //         rotateUp = false;
    //     } else {
    //         rotateUp = true;
    //     }
    // }
    // lastLeftClickState = glfwGetMouseButton(window, 0);
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
}



class Element {
    public:
        unsigned int VAO, VBO, EBO;
        float x,y,z;
        bool wireframe = false;
        GLenum draw_mode = GL_TRIANGLES;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        
        Element() = default;

        Element(const Element& other)
            : x(other.x), y(other.y), z(other.z),
              vertices(other.vertices), indices(other.indices) {
            init(); // create new VAO/VBO/EBO
        }

        void init() {
            glGenVertexArrays(1, &VAO);
            glBindVertexArray(VAO);

            glGenBuffers(1, &VBO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

            glGenBuffers(1, &EBO);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0); 
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
            glEnableVertexAttribArray(1);
        };
        void draw() const {
            if (wireframe == true)
                glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
            glBindVertexArray(VAO);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
            glDrawElements(draw_mode, indices.size(), GL_UNSIGNED_INT, 0);
            if (wireframe == true)
                glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        };

        bool rotate = false;
        glm::vec3 rotateAxis = glm::vec3(0.0f, 1.0f, 0.0f); // what axis(es) to apply rotation to
        float rotationSpeed = 0.0f; // the speed at which we rotate
        glm::vec3 pivot = glm::vec3(0.0f); // what point to rotate around

        // if rotate is true, the element will rotate along rotateAxis axises, at rotationSpeed speed, around point pivot.
        // pivot is the point RELATIVE TO THE MODEL, not the world

        float currentAngle = 0.0f; // to track rotation over time
        void update(float deltaTime) { // deltaTime is how long since last frame and current (i think)
            if (rotate) {
                currentAngle += rotationSpeed * deltaTime;
                if (currentAngle > glm::two_pi<float>()) currentAngle -= glm::two_pi<float>();
            }
        }
        
        glm::mat4 getModelMatrix() const {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(x, y, z));
            if (rotate) {
                model = glm::translate(model, pivot);
                model = glm::rotate(model, currentAngle, rotateAxis);
                model = glm::translate(model, -pivot);
            }
            return model;
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

    // shader shit
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if(!success) {
        glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << "\n";
    }

    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if(!success) {
        glGetShaderInfoLog(fragmentShader,512,NULL,infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT:COMPILATION_FAILED\n" << infoLog << "\n";
    }

    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();

    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::FAILED\n" << infoLog << "\n";
    }

    glUseProgram(shaderProgram);

    std::vector<Element*> Objects;
    Element quad;
    quad.vertices = {
        -5.0f, -0.5f, -5.0f,  1.0f, 0.0f, 0.0f,
        5.0f, -0.5f, -5.0f,   0.0f, 1.0f, 0.0f,
        5.0f, -0.5f, 5.0f,    0.0f, 0.0f, 1.0f,
        -5.0f, -0.5f, 5.0f,   0.0f, 1.0f, 0.0f,
    };
    quad.indices = {
        0, 1, 2,
        2, 3, 0
    };
    quad.x = 0.0f;
    quad.y = 0.0f;
    quad.init();
    // Objects.push_back(&quad);

    // x,y,z
    Element indicator;
    indicator.vertices = {
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // 0
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f, // 1

        0.0f, 0.0f, 0.0f,   0.0f, 1.0f, 0.0f, // 2
        0.0f, 1.0f, 0.0f,   0.0f, 1.0f, 0.0f, // 3

        0.0f, 0.0f, 0.0f,   0.0f, 0.0f, 1.0f, // 4
        0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 1.0f  // 5
    };
    indicator.indices = {
        0, 1,
        2, 3,
        4, 5
    };
    indicator.draw_mode = GL_LINES;
    indicator.init();
    Objects.push_back(&indicator);

    Element spinning_line;
    spinning_line.vertices = {
        0.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 0.0f,   1.0f, 0.0f, 0.0f
    };
    spinning_line.indices = {
        0, 1
    };
    spinning_line.x = 0;
    spinning_line.z = 0;
    spinning_line.y = 0;
    spinning_line.draw_mode = GL_LINES;
    spinning_line.rotate = true;
    spinning_line.rotateAxis = glm::vec3(1.0f, 1.0f, 1.0f); // what axis(es) to apply rotation to
    spinning_line.rotationSpeed = 5.0f; // the speed at which we rotate
    spinning_line.pivot = glm::vec3(0.0f); // what point to rotate around
    spinning_line.init();
    Objects.push_back(&spinning_line);


    glUseProgram(shaderProgram);
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
        

        // glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        for (Element* e : Objects) {
            e->update(deltaTime);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(e->getModelMatrix()));
            e->draw();
        };

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteProgram(shaderProgram);
    glfwTerminate();

    return 0;
}