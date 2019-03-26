#include "PointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 color, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular,float constant, float linear, float quadratic)
{
    this->position = position;
    this->color = color;
    this->ambient = ambient;
    this->diffuse = diffuse;
    this->specular = specular;
    if (constant <= 0)
        constant = 1.0;
    if (linear < 0)
        linear = 1.0;
    if (quadratic < 0)
        quadratic = 1.0;
    this->constant = constant;
    this->linear = linear;
    this->quadratic = quadratic;
}