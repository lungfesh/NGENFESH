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
        return true;
    }
    return false;
}
glm::vec3 castRay(glm::vec3 origin, glm::vec3 direction, float rayLength, const std::vector<Element*> Objects) { // this is making me think suicidal thoughts
    direction = glm::normalize(direction);
    glm::vec3 hitPoint = glm::vec3(0.0f,0.0f,0.0f);
    float closestT = rayLength;

    for (size_t i = 0; i < Objects.size(); i++) {
        Element* obj = Objects[i];

        glm::vec3 t1 = (obj->bounding_box_corner1 - origin) / direction;
        glm::vec3 t2 = (obj->bounding_box_corner2 - origin) / direction;

        glm::vec3 tMin = glm::min(t1, t2);
        glm::vec3 tMax = glm::max(t1, t2);

        float tNear = std::max({tMin.x, tMin.y, tMin.z});
        float tFar  = std::min({tMax.x, tMax.y, tMax.z});

        if (tNear <= tFar && tNear < closestT && tNear >= 0.0f) {
            closestT = tNear;
            hitPoint = origin + direction * tNear;
        }
    }

    return hitPoint;
}

std::vector<float> calcBoundingBoxVerts(glm::vec3 c1, glm::vec3 c2, glm::vec3 color, bool debug) {
    std::vector<float> vertices;
    
    float minX = std::min(c1.x, c2.x);
    float maxX = std::max(c1.x, c2.x);
    float minY = std::min(c1.y, c2.y);
    float maxY = std::max(c1.y, c2.y);
    float minZ = std::min(c1.z, c2.z);
    float maxZ = std::max(c1.z, c2.z);

    // Bottom face
    vertices.insert(vertices.end(), {minX, minY, minZ, color.r,color.g,color.b});
    if (!debug) {
        vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    }
    vertices.insert(vertices.end(), {maxX, minY, minZ, color.r,color.g,color.b});
    if (!debug) {
        vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    }
    vertices.insert(vertices.end(), {maxX, maxY, minZ, color.r,color.g,color.b});
    if (!debug) {
        vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    }
    vertices.insert(vertices.end(), {minX, maxY, minZ, color.r,color.g,color.b});
    if (!debug) {
        vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    }

    // Top face
    vertices.insert(vertices.end(), {minX, minY, maxZ, color.r,color.g,color.b});
    if (!debug) {
        vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    }
    vertices.insert(vertices.end(), {maxX, minY, maxZ, color.r,color.g,color.b});
    if (!debug) {
        vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    }
    vertices.insert(vertices.end(), {maxX, maxY, maxZ, color.r,color.g,color.b});
    if (!debug) {
        vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    }
    vertices.insert(vertices.end(), {minX, maxY, maxZ, color.r,color.g,color.b});
    if (!debug) {
        vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    }
    return vertices;
}

void drawDebugLine(glm::vec3 pos1, glm::vec3 pos2, glm::vec3 color, Shader debugShader,const glm::mat4& view, const glm::mat4& projection) {
    std::vector<float> lineVertices = {
        pos1.x, pos1.y, pos1.z, 1.0f, 1.0f, 1.0f,
        pos2.x, pos2.y, pos2.z, 1.0f, 1.0f, 1.0f
    };
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, lineVertices.size() * sizeof(float), lineVertices.data(), GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glDisable(GL_DEPTH_TEST);
    debugShader.use();
    debugShader.setMat4("view", view);
    debugShader.setMat4("projection", projection);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, 2);
    glEnable(GL_DEPTH_TEST);
}