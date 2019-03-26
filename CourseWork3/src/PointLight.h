#ifndef POINT_LIGHT_H
#define POINT_LIGHT_H

#include "Light.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class PointLight : public Light
{
public:
    PointLight(glm::vec3 position, glm::vec3 color, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic);       
    glm::vec3 getPosition() { return position; }    
    void setPosition(const glm::vec3& pos) { position = pos; }      
    float getConstant() { return constant; }
    float getLinear() { return linear; }
    float getQuadratic() { return quadratic; }
    void setConstant(float constant) { this->constant = (constant > 0) ? constant : 1.0; }
    void setLinear(float linear) { this->linear = (linear >= 0) ? linear : 1.0; }
    void setQuadratic(float quadratic) { this->quadratic = (quadratic >= 0)? quadratic : 1.0; }
 private:
    glm::vec3 position;    
    float constant;
    float linear;
    float quadratic;
};

#endif // !POINT_LIGHT_H

