#include "PointLight.h"

PointLight::PointLight(glm::vec3 position, glm::vec3 color
    // , glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular
    , float constant, float linear, float quadratic):
    Light(color/*, ambient, diffuse, specular*/)
{
    _position = position;
    if (constant <= 0)
        constant = 1.0;
    if (linear < 0)
        linear = 1.0;
    if (quadratic < 0)
        quadratic = 1.0;

    _constant = constant;
    _linear = linear;
    _quadratic = quadratic;
}