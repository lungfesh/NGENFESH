#include <glm/glm.hpp>
#include "util.hpp"
#include <math.h>
#include <algorithm>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

// given origin, direction, loop thru Element list and check if it hit anything and return where it hit
glm::vec3 castRay(glm::vec3 origin, glm::vec3 direction, const std::vector<Element> Objects) {
    for (size_t i = 0; i < Objects.size(); i++) {
        if (Objects[i].isPlayer) continue;
        float tx1 = (Objects[i].bounding_box_corner1.x - origin.x) / direction.x;
        float tx2 = (Objects[i].bounding_box_corner2.x - origin.x) / direction.x;
        float tmin_x = std::min(tx1,tx2);
        float tmax_x = std::max(tx1,tx2);

        float ty1 = (Objects[i].bounding_box_corner1.y - origin.y) / direction.y;
        float ty2 = (Objects[i].bounding_box_corner2.y - origin.y) / direction.y;
        float tmin_y = std::min(ty1,ty2);
        float tmax_y = std::max(ty1,ty2);

        float tz1 = (Objects[i].bounding_box_corner1.z - origin.z) / direction.z;
        float tz2 = (Objects[i].bounding_box_corner2.z - origin.z) / direction.z;
        float tmin_z = std::min(tz1,tz2);
        float tmax_z = std::max(tz1,tz2);

        float t_enter = std::max(std::max(tmin_x, tmin_y), tmin_z);
        float t_exit = std::min(std::max(tmax_x, tmax_y), tmax_z);

        if (t_exit >= t_enter && t_exit >= 0) {
            return origin + direction * t_enter;
        }
    }
    return glm::vec3();
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
