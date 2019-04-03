#ifndef OBJECT_H
#define OBJECT_H

#include "Model.h"
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

class Object
{
public:    

    Object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, Model& model):
        _model(model), 
        _position(position),
        _scale(scale) {}

    Model& getModel() { return _model; }

    void setModel(const Model& model) { _model = model; };

    glm::vec3 getPosition() { return _position; }

    void setPosition(glm::vec3 position) { _position = position; }

    glm::vec3 getScale() { return _scale; }

    void setScale(glm::vec3 scale) { _scale = scale; }

    // Returns translated, rotated and scaled model matrix
    glm::mat4 getModelMatrix();

private:
    //TODO: change Model& to unique_ptr<Model>
    Model& _model;   
    glm::vec3 _position;
    glm::vec3 _rotation;
    glm::vec3 _scale;
};

#endif