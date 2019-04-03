#include "DirectionalLight.h"

DirectionalLight::DirectionalLight(
	glm::vec3 direction, glm::vec3 color, 
	glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
:Light(color, ambient, diffuse, specular)
{
    _direction = direction;
}