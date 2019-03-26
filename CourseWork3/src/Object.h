#ifndef OBJECT_H
#define OBJECT_H

#include "Model.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Object
{
public:    

    Object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, Model& model);

    Model& getModel() { return model; }

    void setModel(const Model& model) { this->model = model; };

    glm::vec3 getPosition() { return position; }

    void setPosition(glm::vec3 position) { this->position = position; }

    glm::vec3 getScale() { return scale; }

    void setScale(glm::vec3 scale) { this->scale = scale; }

    glm::mat4 getModelMatrix();

private:
    Model& model;   
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
};

#endif

