#include <glm/glm.hpp>
#include "element.hpp"

// given 2 bounding boxes, check if they collide using the AABB method
bool AABBCollideDetect(glm::vec3 bounding_box_corner1, glm::vec3 bounding_box_corner2, glm::vec3 bounding_box_corner3, glm::vec3 bounding_box_corner4);

std::vector<float> calcBoundingBoxVerts(glm::vec3 c1, glm::vec3 c2, glm::vec3 color = glm::vec3(1.0f), bool debug = false);
glm::vec3 castRay(glm::vec3 origin, glm::vec3 direction, float rayLength, const std::vector<Element*> Objects);
void drawDebugLine(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color, Shader debugShader,const glm::mat4& view, const glm::mat4& projection);
// compliation takes awhile, might be better to put contents of util.cpp in here