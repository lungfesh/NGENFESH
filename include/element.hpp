#ifndef ELEMENT_HPP
#define ELEMENT_HPP
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <math.h>
#include <random>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/glm.hpp>

#include "texture.hpp"
#include "shader.hpp"
#include "camera.hpp"

class Element {
    public:
        unsigned int VAO = 0, VBO = 0, EBO = 0;
        glm::vec3 position{0.0f};
        glm::vec3 lastPosition{0.0f};
        glm::vec3 velocity{0.0f};
        bool wireframe = false;
        GLenum draw_mode = GL_TRIANGLES;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::string textureFile = "";
        bool useTexture = false;
        Texture texture;

        Shader* shader = nullptr;
        glm::vec3 lightColor;

        // bounding_box_corner1 is min, bounding_box_corner2 is max
        // ALWAYS make sure min is lower (y position is less than) max
        glm::vec3 bounding_box_corner1{-0.5}; // for a 1x1 unit cube
        glm::vec3 bounding_box_corner2{0.5};
        
        float sizex = 1.0f;
        float sizey = 1.0f;
        float sizez = 1.0f;

        Camera* attachedCamera;

        int id = NAN;

        // PHYSICS
        // by default: all has collision, all can have velocity
        // if it hits another thing, it stops
        // no bounce
        bool anchored = false; // velocity does not affect element, cannot be moved
        bool bounce = false;
        float bounce_amount = 0.5f; // how much energy to lose, default at 50%
        bool hasCollision = true;

        Element* debugElement = nullptr;
        bool debug = false;
        Element() = default;

        bool isPlayer = false;

        bool rotate = false;
        glm::vec3 rotateAxis = glm::vec3(0.0f, 1.0f, 0.0f); // what axis(es) to apply rotation to
        float rotationSpeed = 0.0f; // the speed at which we rotate
        glm::vec3 pivot = glm::vec3(0.0f); // what point to rotate around
        glm::vec3 rotation = glm::vec3(0.0f); // initial orientation

        float currentAngle = 0.0f; // to track rotation over time

        void init();
        void draw(const glm::mat4& view, const glm::mat4& projection, Element& lightSource, glm::vec3 cameraPos, float time) const;
        void update(float deltaTime, std::vector<Element*>& Objects);
        glm::mat4 getMatrix() const;
        bool getUseTexture() const;
        void physics_step(float dt);
        ~Element();
        bool grounded = false; // only for player, check for if on ground and let player jump if so.
};


class HUDElement {
    public:
        unsigned int VAO = 0, VBO = 0, EBO = 0;
        glm::vec3 position{0.0f};
        bool wireframe = false;
        GLenum draw_mode = GL_TRIANGLES;

        std::vector<float> vertices;
        std::vector<unsigned int> indices;
        std::string textureFile = "";
        bool useTexture = false;
        Texture texture;

        Shader* shader = nullptr;
        glm::vec3 lightColor;

        HUDElement* debugElement = nullptr;
        bool debug = false;
        HUDElement() = default;

        void init();
        void draw() const;
        void update(float deltaTime);
        bool getUseTexture() const;
        ~HUDElement();

        bool rotate = false;
        glm::vec2 rotateAxis = glm::vec2(0.0f, 1.0f); // what axis(es) to apply rotation to
        float rotationSpeed = 0.0f; // the speed at which we rotate
        glm::vec2 pivot = glm::vec2(0.0f); // what point to rotate around
        glm::vec2 rotation = glm::vec2(0.0f); // initial orientation

        float currentAngle = 0.0f; // to track rotation over time
};

// returns id
int addToWorld(Element* e, std::vector<Element*>& Objects);

#endif