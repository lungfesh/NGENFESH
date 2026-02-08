#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
    glm::vec3 pos{0.0f, 1.0f, 0.0f};
    glm::vec3 front{0.0f, 0.0f, 0.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};

    float yaw = 0.0f;
    float pitch = 0.0f;
    float speed = 5.0f;
    float sensitivity = 0.1f;

    // https://www.youtube.com/watch?v=cFHX5gSMjTs
    // https://registry.khronos.org/OpenGL-Refpages/gl2.1/xhtml/gluLookAt.xml
    glm::mat4 view() const { // view matrix
        return glm::lookAt(pos,pos+front,up);
    }
};
