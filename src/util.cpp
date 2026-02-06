#include <glm/glm.hpp>
#include "util.hpp"

// given 2 bounding boxes, check if they collide using the AABB method
bool AABBCollideDetect(glm::vec3 bounding_box_corner1, glm::vec3 bounding_box_corner2, glm::vec3 bounding_box_corner3, glm::vec3 bounding_box_corner4) { // c1 = min c2 = max
    if (bounding_box_corner1.x < bounding_box_corner4.x &&
        bounding_box_corner2.x > bounding_box_corner3.x &&

        bounding_box_corner1.y < bounding_box_corner4.y &&
        bounding_box_corner2.y > bounding_box_corner3.y &&
        bounding_box_corner1.z < bounding_box_corner4.z &&
        bounding_box_corner2.z > bounding_box_corner3.z
    ) {
        // std::cout << "Collision at " << glfwGetTime() << std::endl;
        return true;
    }
    return false;
}

std::vector<float> calcBoundingBoxVerts(glm::vec3 c1, glm::vec3 c2, glm::vec3 color) {
    std::vector<float> vertices;
    for (int x = 0; x <= 1; ++x) {
        for (int y = 0; y <= 1; ++y) {
            for (int z = 0; z <= 1; ++z) {
                vertices.push_back(x ? c1.x : c2.x);
                vertices.push_back(y ? c1.y : c2.y);
                vertices.push_back(z ? c1.z : c2.z);
                vertices.push_back(color.r);
                vertices.push_back(color.g);
                vertices.push_back(color.b);
                
                // UV
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);

                // Normal
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
                vertices.push_back(0.0f);
            }
        }
    }
    return vertices;
}
