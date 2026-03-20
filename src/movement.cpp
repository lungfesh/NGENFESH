#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "util.hpp"
#include "element.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// look at player move state (freecam/fps), move according to action
// void movePlayer(Element player, glm::vec3 moveDir) {
//     glm::vec3 moveDir(0.0f);
//     if (action == 0) {
//         moveDir += player.attachedCamera.front;
//         player.velocity += player.attachedCamera.front
//     }
// }