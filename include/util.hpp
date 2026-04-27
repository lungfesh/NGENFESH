#ifndef UTIL_HPP
#define UTIL_HPP
#include <glm/glm.hpp>
#include "element.hpp"

// given 2 bounding boxes, check if they collide using the AABB method
bool AABBCollideDetect(glm::vec3 bounding_box_corner1, glm::vec3 bounding_box_corner2, glm::vec3 bounding_box_corner3, glm::vec3 bounding_box_corner4);

std::vector<float> calcBoundingBoxVerts(glm::vec3 c1, glm::vec3 c2, glm::vec3 color = glm::vec3(1.0f), bool debug = false);
glm::uvec2 newDebugLine();
void drawDebugLine(unsigned int VAO, unsigned int VBO, glm::vec3 origin, glm::vec3 direction, float length, Shader& debugShader, const glm::mat4& view, const glm::mat4& projection);

// given 3 vertices, return normal vector
inline glm::vec3 calcNormal(glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
    return glm::normalize(glm::cross((v1 - v0), (v2 - v0)));
}
// given list of vertices, return min and max
std::vector<glm::vec3> calcBoundingBoxPoints(std::vector<float> vertices);

struct Rayhit {
    Element* hitElement = nullptr;
    glm::vec3 hitPos = glm::vec3(NAN);
    float distance = FLT_MAX;
};

Rayhit Raycast(glm::vec3 origin, glm::vec3 direction, std::vector<Element*>& Objects, Element* caster = nullptr);

#endif