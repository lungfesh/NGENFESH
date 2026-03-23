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
    playerElement.wireframe = true;
    playerElement.position = glm::vec3(0.0f,5.0f,0.0f);
    playerElement.bounding_box_corner1 = glm::vec3(-0.5);
    playerElement.bounding_box_corner2 = glm::vec3(0.5f, 1.5f, 0.5f);
    playerElement.vertices = calcBoundingBoxVerts(playerElement.bounding_box_corner1, playerElement.bounding_box_corner2, glm::vec3(1.0f,0.0f,0.0f));
    playerElement.indices = {CUBE_INDICES};
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
    attachedCamera->setFront(playerState.cameraOrientation);
    playerState.velocity = playerElement.velocity;
    orient(attachedCamera->getYaw(), attachedCamera->getPitch());
    // attachedCamera->setPos(playerState.position);
    // orient(playerState.cameraOrientation)
}

void Player::keyInput() {

}

void Player::orient(float yaw, float pitch) { // sets to yaw and pitch
    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    this->camera()->setFront(glm::normalize(direction));
}