#include "Object.h"

using namespace std;

Object::Object(glm::vec3 position, glm::vec3 rotation, glm::vec3 scale, Model& model):model(model)
{
    this->position = position;
    this->rotation = rotation;
    this->scale = scale;    
}

glm::mat4 Object::getModelMatrix()
{
    glm::mat4 model = glm::mat4();
    model = glm::translate(model, position);
    glm::quat quaternion(glm::vec3(glm::radians(rotation.x), glm::radians(rotation.y), glm::radians(rotation.z)));
    model = model * toMat4(quaternion);
    model = glm::scale(model, scale);
    return model;
}