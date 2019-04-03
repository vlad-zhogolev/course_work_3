#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "Light.h"

class DirectionalLight : public Light
{
public:
    DirectionalLight(
    	glm::vec3 direction, glm::vec3 color, 
    	glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular);
    
    glm::vec3 getDirection() { return _direction; }
   
    void setDirection(glm::vec3 direction) { _direction = direction; }
    
private:
    glm::vec3 _direction;
};

#endif