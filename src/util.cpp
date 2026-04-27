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
    // if (!debug) {
    //     vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    // }
    vertices.insert(vertices.end(), {maxX, minY, minZ, color.r,color.g,color.b});
    // if (!debug) {
    //     vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    // }
    vertices.insert(vertices.end(), {maxX, maxY, minZ, color.r,color.g,color.b});
    // if (!debug) {
    //     vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    // }
    vertices.insert(vertices.end(), {minX, maxY, minZ, color.r,color.g,color.b});
    // if (!debug) {
    //     vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    // }

    // Top face
    vertices.insert(vertices.end(), {minX, minY, maxZ, color.r,color.g,color.b});
    // if (!debug) {
    //     vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    // }
    vertices.insert(vertices.end(), {maxX, minY, maxZ, color.r,color.g,color.b});
    // if (!debug) {
    //     vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    // }
    vertices.insert(vertices.end(), {maxX, maxY, maxZ, color.r,color.g,color.b});
    // if (!debug) {
    //     vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    // }
    vertices.insert(vertices.end(), {minX, maxY, maxZ, color.r,color.g,color.b});
    // if (!debug) {
    //     vertices.insert(vertices.end(), {0.0f,0.0f, 0.0f,0.0f,0.0f});
    // }
    return vertices;
}
// returns ID of VAO,VBO of new debug line
glm::uvec2 newDebugLine() {
    unsigned int debugVAO;
    unsigned int debugVBO; 
    glGenVertexArrays(1, &debugVAO);
    glGenBuffers(1, &debugVBO);

    glBindVertexArray(debugVAO);
    glBindBuffer(GL_ARRAY_BUFFER, debugVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 12, nullptr, GL_DYNAMIC_DRAW); // GL_DYNAMIC_DRAW means we are updating this often, probably not every frame though

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    return glm::uvec2(debugVAO, debugVBO);
}
void drawDebugLine(unsigned int VAO, unsigned int VBO, glm::vec3 origin, glm::vec3 direction, float length, Shader& debugShader, const glm::mat4& view, const glm::mat4& projection) {
    direction = glm::normalize(direction);
    glm::vec3 color = glm::abs(direction);
    std::vector<float> lineVertices = {
        origin.x, origin.y, origin.z, color.x, color.y, color.z,
        origin.x + (length * direction.x), origin.y + (length * direction.y), origin.z + (length * direction.z), color.x, color.y, color.z
    };

    // glDisable(GL_DEPTH_TEST);
    glLineWidth(0.5f);
    debugShader.use();
    debugShader.setMat4("view", view);
    debugShader.setMat4("projection", projection);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * lineVertices.size(), lineVertices.data()); // so cool!

    glDrawArrays(GL_LINES, 0, 2);
    // glEnable(GL_DEPTH_TEST);
}
// we are passing entire vertices std::vector in now, we should probably a vector for each element with js the pos coords in the future
std::vector<glm::vec3> calcBoundingBoxPoints(std::vector<float> vertices) {
    glm::vec3 min = glm::vec3(0.0f,0.0f,0.0f);
    glm::vec3 max = glm::vec3(0.0f,0.0f,0.0f);
    for (size_t i = 0; i < vertices.size(); i=i+11) {
        min.x = std::min(min.x, vertices[i+0]);
        min.y = std::min(min.y, vertices[i+1]);
        min.z = std::min(min.z, vertices[i+2]);
    
        max.x = std::max(max.x, vertices[i+0]);
        max.y = std::max(max.y, vertices[i+1]);
        max.z = std::max(max.z, vertices[i+2]);
    }
    return std::vector<glm::vec3> {
        min, max
    };
}

Rayhit Raycast(glm::vec3 origin, glm::vec3 direction, std::vector<Element*>& Objects, Element* caster) { // https://gdbooks.gitbooks.io/3dcollisions/content/Chapter3/raycast_aabb.html
    Rayhit hit;
    float closest = FLT_MAX;
    for (size_t i = 0; i < Objects.size(); i++) {
        if (Objects[i]->debug == true) {continue;} // do not collide with debug elements
        if (!Objects[i]->hasCollision) {continue;}
        if (Objects[i] == caster) {continue;}

        glm::vec3 min = Objects[i]->position + Objects[i]->bounding_box_corner1;
        glm::vec3 max = Objects[i]->position + Objects[i]->bounding_box_corner2;
        
        float t1 = (min.x - origin.x) / direction.x;
        float t2 = (max.x - origin.x) / direction.x;
        float t3 = (min.y - origin.y) / direction.y;
        float t4 = (max.y - origin.y) / direction.y;
        float t5 = (min.z - origin.z) / direction.z;
        float t6 = (max.z - origin.z) / direction.z;

        float tmin = glm::max(glm::max(glm::min(t1, t2), glm::min(t3, t4)), glm::min(t5, t6));
        float tmax = glm::min(glm::min(glm::max(t1, t2), glm::max(t3, t4)), glm::max(t5, t6));

        if (tmax < 0.0f || tmin > tmax) continue;

        float t = (tmin >= 0.0f) ? tmin : tmax;

        if (t < hit.distance) {
            hit.distance = t;
            hit.hitElement = Objects[i];
        }
        // return tmin;
    }
    return hit;
}