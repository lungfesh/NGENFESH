#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.hpp"
#include "camera.hpp"
#include "texture.hpp"
#include "util.hpp"
#include "element.hpp"
void Element::init() {
    if (useTexture)
        texture.init(textureFile);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    if (!debug) {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6*sizeof(float)));
        glEnableVertexAttribArray(2);

        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8*sizeof(float)));
        glEnableVertexAttribArray(3);
    } else {
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
        glEnableVertexAttribArray(1);

        // glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(6*sizeof(float)));
        // glEnableVertexAttribArray(2);

        // glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(8*sizeof(float)));
        // glEnableVertexAttribArray(3);
    }
};
void Element::draw(const glm::mat4& view, const glm::mat4& projection, Element& lightSource, glm::vec3 cameraPos, float time) const {
    if (!shader) return;


    shader->use();
    shader->setMat4("view", view);
    shader->setMat4("projection", projection);
    shader->setMat4("model", getMatrix());
    shader->setInt("useTexture", getUseTexture());
    shader->setVec3("lightColor", lightSource.lightColor);
    shader->setVec3("lightPos", glm::vec3(lightSource.position.x, lightSource.position.y, lightSource.position.z));
    shader->setVec3("viewPos", cameraPos);
    if (wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    if (useTexture)
        texture.use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(draw_mode, indices.size(), GL_UNSIGNED_INT, 0);
    if (wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    if (useTexture)
        texture.unUse();
};
void Element::update(float deltaTime, std::vector<Element*>& Objects) { // deltaTime is how long since last frame and current (i think)
    if (debugElement != nullptr)
        debugElement->position = position;
    if (rotate) {
        currentAngle += rotationSpeed * deltaTime;
        if (currentAngle > glm::two_pi<float>()) currentAngle -= glm::two_pi<float>();
    }
    if (lastPosition != position && (hasCollision)) {
        for (size_t i = 0; i < Objects.size(); i++) {
            if (Objects[i] == this) {continue;}
            if (Objects[i]->debug == true) {continue;} // do not collide with debug elements
            if (Objects[i]->position == position) {continue;} // oh the horrors
            if (AABBCollideDetect(position+bounding_box_corner1,
                position+bounding_box_corner2,
                Objects[i]->position+Objects[i]->bounding_box_corner1, 
                Objects[i]->position+Objects[i]->bounding_box_corner2)) {
                // printf("Collision at %f, pos1: %f %f %f, pos2: %f %f %f\n", glfwGetTime(),Objects[i]->position.x, Objects[i]->position.y, Objects[i]->position.z, position.x, position.y, position.z);
                float px = std::min(position.x+bounding_box_corner2.x, Objects[i]->position.x+Objects[i]->bounding_box_corner2.x) - std::max(position.x+bounding_box_corner1.x, Objects[i]->position.x+Objects[i]->bounding_box_corner1.x);
                float py = std::min(position.y+bounding_box_corner2.y, Objects[i]->position.y+Objects[i]->bounding_box_corner2.y) - std::max(position.y+bounding_box_corner1.y, Objects[i]->position.y+Objects[i]->bounding_box_corner1.y);
                float pz = std::min(position.z+bounding_box_corner2.z, Objects[i]->position.z+Objects[i]->bounding_box_corner2.z) - std::max(position.z+bounding_box_corner1.z, Objects[i]->position.z+Objects[i]->bounding_box_corner1.z);
                // thanks chatgpt
                // we're checking which axis has the most overlap, setting pos of 1 object to not be inside the other, setting vel on that axis to 0
                // this should probably have it's own func
                // if (glm::length(velocity) == 0.0f) return;
                if (bounce) {
                    if (px < py && px < pz) { // z axis has most overlap
                        float dir = (position.x < Objects[i]->position.x) ? -1.0f : 1.0f;
                        position.x += px * dir;
                        velocity.x = -velocity.x * bounce_amount;
                        if (glm::abs(velocity.x) < 0.08f) velocity.x = 0.0f;
                    }
                    else if (py < pz) { // y axis has most overlap
                        float dir = (position.y < Objects[i]->position.y) ? -1.0f : 1.0f;
                        position.y += py * dir;
                        velocity.y = -velocity.y * bounce_amount;
                        if (glm::abs(velocity.y) < 0.08f) velocity.y = 0.0f;
                    }
                    else { // x axis overlap
                        float dir = (position.z < Objects[i]->position.z) ? -1.0f : 1.0f;
                        position.z += pz * dir;
                        velocity.z = -velocity.z;
                        if (glm::abs(velocity.z) < 0.08f) velocity.z = 0.0f;
                    }
                    } else {
                    if (px < py && px < pz) {
                        float dir = (position.x < Objects[i]->position.x) ? -1.0f : 1.0f;
                        position.x += px * dir;
                        Objects[i]->velocity.x = velocity.x;
                        velocity.x = 0;
                    } else if (py < pz) {
                        float dir = (position.y < Objects[i]->position.y) ? -1.0f : 1.0f;
                        position.y += py * dir;
                        Objects[i]->velocity.y = velocity.y;
                        velocity.y = 0;
                    } else {
                        float dir = (position.z < Objects[i]->position.z) ? -1.0f : 1.0f;
                        position.z += pz * dir;
                        Objects[i]->velocity.z = velocity.z;
                        velocity.z = 0;
                    }
                }
            }
        }
    }
    // check if element is grounded (on top of another object)
    float epsilon = 0.01f; // small tolerance
    float groundRayLength = bounding_box_corner1.y - position.y;
    groundRay = castRay(position, glm::vec3(0.0f, -1.0f, 0.0f), groundRayLength, Objects);

    if (!glm::isnan(groundRay.y)) {
        float bottomY = position.y + bounding_box_corner1.y;
        grounded = (bottomY - groundRay.y) <= epsilon;
    } else {
        grounded = false;
    }
    
    lastPosition = position;
    if (isPlayer) {
        printf("%f %f %f\n", position.x, position.y, position.z);
    }
}
        
glm::mat4 Element::getMatrix() const { // get model matrix
    glm::mat4 model = glm::mat4(1.0f); // define 4x4 matrix
    model = glm::translate(model, position); // add position translation to matrix
    model = glm::scale(model, glm::vec3(sizex, sizey, sizez));
    if (rotate) { // if we're rotating it
        model = glm::translate(model, pivot); // translate first by pivot point
        model = glm::rotate(model, rotation.x, glm::vec3(1, 0, 0)); // rotate on x,y,z
        model = glm::rotate(model, rotation.y, glm::vec3(0, 1, 0));
        model = glm::rotate(model, rotation.z, glm::vec3(0, 0, 1));
        model = glm::rotate(model, currentAngle, rotateAxis); // if continuosly rotating, rotate by current angle on rotate axis
        model = glm::translate(model, -pivot); // translate by negative pivot point, no idea why we do this i think i stole this from somewhere
    }
    return model;
}

bool Element::getUseTexture() const {
    return useTexture;
}

void Element::physics_step(float dt) {
        if (anchored) return;
        velocity.y += -9.8*dt;

    // now dampen so it doesn't fly forever
    float damping = 2.0f; // units per second
    if (glm::length(velocity) > 0.0f) {
        glm::vec3 decel = glm::normalize(velocity) * damping * dt;
        if (glm::length(decel) > glm::length(velocity))
            velocity = glm::vec3(0.0f); // stop completely
        else {
            velocity -= decel;
        }
    }
    if (glm::length(velocity) < 0.1f) {
        velocity = glm::vec3(0.0f);
    }
    position += velocity * dt; // apply velocity
}

Element::~Element() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}


void HUDElement::init() {
    if (useTexture)
        texture.init(textureFile);
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glGenBuffers(1, &EBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);
};
void HUDElement::draw() const {
    if (!shader) return;
    shader->use();
    shader->setInt("useTexture", getUseTexture());
    if (wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    if (useTexture)
        texture.use();
    glBindVertexArray(VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glDrawElements(draw_mode, indices.size(), GL_UNSIGNED_INT, 0);
    if (wireframe)
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
    if (useTexture)
        texture.unUse();
};
void HUDElement::update(float deltaTime) { // deltaTime is how long since last frame and current (i think)
    if (rotate) {
        currentAngle += rotationSpeed * deltaTime;
        if (currentAngle > glm::two_pi<float>()) currentAngle -= glm::two_pi<float>();
    }
}

bool HUDElement::getUseTexture() const {
    return useTexture;
}

HUDElement::~HUDElement() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
}