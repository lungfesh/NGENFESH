#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "util.hpp"
#include "element.hpp"
#include "player.hpp"
#include "premade_elements.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

void Player::init(std::vector<Element*>& Objects, Camera* cam) {
    attachedCamera = cam;
    // if null, create camera for player
    if (!attachedCamera || attachedCamera == NULL) {
        printf("player.cpp: Camera is null!\n");
        return;
    }
    printf("player.cpp: Creating element for player...\n");
    // create Element for player
    // playerElement.wireframe = true;
    playerElement.position = glm::vec3(0.0f,5.0f,0.0f);
    playerElement.bounding_box_corner1 = glm::vec3(-0.5);
    playerElement.bounding_box_corner2 = glm::vec3(0.5f, 1.5f, 0.5f);
    // playerElement.vertices = calcBoundingBoxVerts(playerElement.bounding_box_corner1, playerElement.bounding_box_corner2, glm::vec3(1.0f,0.0f,0.0f));
    // playerElement.indices = {CUBE_INDICES};
    playerElement.anchored = false;
    playerElement.hasCollision = true;
    playerElement.isPlayer = true;
    playerElement.attachedCamera = attachedCamera;
    playerElement.init();
    if (attachedCamera == nullptr || attachedCamera == NULL) {
        printf("player.cpp: attachCamera is nullptr!\n");
        return;
    }
    addToWorld(&playerElement, Objects);
};

void Player::update() {
    attachedCamera->update();
    playerState.position = playerElement.position;
    attachedCamera->setPos(playerState.position + attachedCamera->getOffset());
    playerState.cameraOrientation = camera()->getOrientation();
    playerState.velocity = playerElement.velocity;
    orient(attachedCamera->getYaw(), attachedCamera->getPitch());

    playerState.moveState = (playerElement.grounded) ? 'g' : 'a';
    // printf("player is %s\n", (playerState.moveState == 'g') ? "grounded" : "in air");
    // printf("velocity: %f %f %f\n", getVelocityX(), getVelocityY(), getVelocityZ());
    // printf("total speed: %f\n----\n", glm::sqrt(getVelocityX()*getVelocityX() + getVelocityY()*getVelocityY() + getVelocityZ()*getVelocityZ()));
}

void Player::keyInput(GLFWwindow *window, float deltaTime, std::vector<Element*>& Objects) {
    glm::vec3 right = glm::normalize(glm::cross(camera()->getOrientation(),camera()->getUp()));
    glm::vec3 forward = camera()->getOrientation();
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);
    glm::vec3 moveDir = glm::vec3(0.0f);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) // 0
        {moveDir += forward;}
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        moveDir -= forward;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        moveDir -= right;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        moveDir += right;
    setVelocityX(moveDir.x * playerState.speed*deltaTime);
    setVelocityZ(moveDir.z * playerState.speed*deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && (getMoveState() != 'a')) {
        setVelocityY(playerState.jumpPower);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        Rayhit pickupHit = Raycast(camera()->getPos(), getCameraOrientation(), Objects, &playerElement);
        if (pickupHit.hitElement != nullptr) {
            printf("Hit!\n");
            pickupHit.hitElement->position = getCameraPos() + getCameraOrientation() * 2.0f;
            return;
        }
        printf("No hit!\n");
    }
}

void Player::orient(float yaw, float pitch) { // sets to yaw and pitch
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->camera()->setFront(glm::normalize(direction));
}