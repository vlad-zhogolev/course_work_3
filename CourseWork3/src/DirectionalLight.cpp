#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(glm::vec3 direction, glm::vec3 color, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
{
    this->direction = direction;
    this->color = color;
    this->ambient = ambient;
    this->diffuse = diffuse;
    this->specular = specular;
}