#include <glm/glm.hpp>

// given 2 bounding boxes, check if they collide using the AABB method
bool AABBCollideDetect(glm::vec3 bounding_box_corner1, glm::vec3 bounding_box_corner2, glm::vec3 bounding_box_corner3, glm::vec3 bounding_box_corner4);

std::vector<float> calcBoundingBoxVerts(glm::vec3 c1, glm::vec3 c2, glm::vec3 color = glm::vec3(1.0f,0.0f,0.0f)); 
// compliation takes awhile, might be better to put contents of util.cpp in here