#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
    private:
        glm::vec3 pos{0.0f, 0.0f, 0.0f};
        glm::vec3 front{0.0f, 0.0f, 0.0f};
        glm::vec3 up{0.0f, 1.0f, 0.0f}; // don't change this
        glm::vec3 offset{0.0f, 1.0f, 0.0f};
        float speed = 200.0f; // only for freecam, probably will give to Player class later
        float sensitivity = 0.1f;

        float yaw = 0.0f;
        float pitch = 0.0f;
    public:
        // https://www.youtube.com/watch?v=cFHX5gSMjTs
        // https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml
        glm::mat4 view() const { // view matrix
            return glm::lookAt(pos,pos+front,up);
        }
        void update() {
            if (pitch > 89.9f)
                pitch = 89.9f;
            if (pitch < -89.9f)
                pitch = -89.9f;
        }
        void setPos(glm::vec3 position) {
            pos = position;
        }
        void setFront(glm::vec3 aFront) {
            front = aFront;
        }
        void setPitch(float aPitch) {
            pitch = aPitch;
        }
        void setYaw(float aYaw) {
            yaw = aYaw;
        }
        glm::vec3 getOrientation() {
            return front;
        }
        glm::vec3 getUp() {
            return up;
        }
        glm::vec3 getPos() {
            return pos;
        }
        glm::vec3 getOffset() {
            return offset;
        }
        float getYaw() {return yaw;}
        float getPitch() {return pitch;}
};

#endif