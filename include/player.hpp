#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "util.hpp"
#include "element.hpp"
#include "premade_elements.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Player {
    public:
        void init(std::vector<Element*>& Objects, Camera* cam);
        void update(); // mainly updates playerState
        Camera* camera() {
            return attachedCamera;
        }
        // keyInput() will go through it's binds, move or do whatever if needed 
        void keyInput(GLFWwindow *window, float deltaTime, std::vector<Element*>& Objects);
        void orient(float yaw, float pitch);
        float getSpeed() {
            return playerState.speed;
        }
        glm::vec3 getVelocity() {
            return playerState.velocity;
        }
        glm::vec3 getPosition() {
            return playerState.position;
        }
        void setPosition(glm::vec3 pos) {
            playerElement.position = pos;
        }
        void setVelocity(glm::vec3 aVelocity) {playerElement.velocity = aVelocity;}
        void setVelocityX(float x) {playerElement.velocity.x = x;}
        void setVelocityY(float y) {playerElement.velocity.y = y;}
        void setVelocityZ(float z) {playerElement.velocity.z = z;}
        
        char getMoveState() {return playerState.moveState;}

        float getVelocityX() {return playerState.velocity.x;}
        float getVelocityY() {return playerState.velocity.y;}
        float getVelocityZ() {return playerState.velocity.z;}
        glm::vec3 getCameraOrientation() {return playerState.cameraOrientation;};
        glm::vec3 getCameraPos() {return attachedCamera->getPos();};
        Element playerElement;
        Camera* attachedCamera;

        void attemptPickupElement(std::vector<Element*>& Objects);
    private:
        struct state { // probably gonna want to make some sort of humanoid class with this instead, and just use that
            char moveState = 'g'; // 'a' for grounded, 'a' for in air
            glm::vec3 position;
            glm::vec3 velocity;
            glm::vec3 cameraOrientation;
            float speed = 200.0f;
            float jumpPower = 6.0f;
            bool holdingSomething;
            Element* heldElement;
        };
        state playerState; // do not edit playerstate.pos/vel, this is only for reading
};

#endif