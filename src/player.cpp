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
    if (WorldObjects == nullptr) {
        printf("WorldObjects is not set for player!\n");
        return;
    }
    addToWorld(&playerElement, *WorldObjects);
};

void Player::update() {
    attachedCamera->update();
    playerState.position = playerElement.position;
    attachedCamera->setPos(playerState.position + attachedCamera->getOffset());
    playerState.cameraOrientation = camera()->getOrientation();
    playerState.velocity = playerElement.velocity;
    orient(attachedCamera->getYaw(), attachedCamera->getPitch());

    playerState.moveState = (playerElement.grounded) ? 'g' : 'a';
    if (playerState.holdingSomething) {
        playerState.heldElement->position = getCameraPos() + getCameraOrientation() * 2.0f;
        playerState.heldElement->gravity = false;
    }
    // printf("player is %s\n", (playerState.moveState == 'g') ? "grounded" : "in air");
    // printf("velocity: %f %f %f\n", getVelocityX(), getVelocityY(), getVelocityZ());
    // printf("total speed: %f\n----\n", glm::sqrt(getVelocityX()*getVelocityX() + getVelocityY()*getVelocityY() + getVelocityZ()*getVelocityZ()));
}

void Player::keyInput(float deltaTime, KeyState keys[512]) {
    glm::vec3 right = glm::normalize(glm::cross(camera()->getOrientation(),camera()->getUp()));
    glm::vec3 forward = camera()->getOrientation();
    forward.y = 0.0f;
    if (glm::length(forward) > 0.0f)
        forward = glm::normalize(forward);
    glm::vec3 moveDir = glm::vec3(0.0f);
    
    if (keys[GLFW_KEY_W].currentState)
        moveDir += forward;
    if (keys[GLFW_KEY_S].currentState)
        moveDir -= forward;
    if (keys[GLFW_KEY_A].currentState)
        moveDir -= right;
    if (keys[GLFW_KEY_D].currentState)
        moveDir += right;

    if (glm::length(moveDir) > 0.0f)
        moveDir = glm::normalize(moveDir);

    playerElement.velocity.x = moveDir.x * playerState.speed;
    playerElement.velocity.z = moveDir.z * playerState.speed;
    
    if (keys[GLFW_KEY_SPACE].currentState && getMoveState() == 'g') playerElement.velocity.y += playerState.jumpPower;

    if (keys[GLFW_KEY_E].currentState && !keys[GLFW_KEY_E].pastState) attemptPickupElement();
}

void Player::attemptPickupElement() {
        if (playerState.holdingSomething) {
            playerState.holdingSomething = false;
            playerState.heldElement->gravity = true;
            playerState.heldElement = nullptr;
            return;
        }

        Rayhit pickupHit = Raycast(camera()->getPos(), getCameraOrientation(), *WorldObjects, &playerElement);
        if (pickupHit.hitElement != nullptr) {
            playerState.holdingSomething = true;
            playerState.heldElement = pickupHit.hitElement;
        }
}

void Player::orient(float yaw, float pitch) { // sets to yaw and pitch
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->camera()->setFront(glm::normalize(direction));
}