#ifndef SPOT_LIGHT_H
#define SPOT_LIGHT_H

#include "Light.h"

class SpotLight : public Light
{
public:
    SpotLight(glm::vec3 position, glm::vec3 color, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff);   
    glm::vec3 getPosition() { return position; }    
    void setPosition(const glm::vec3& pos) { position = pos; }
    glm::vec3 getDirection() { return direction; }
    float getConstant() { return constant; }
    float getLinear() { return linear; }
    float getQuadratic() { return quadratic; }
    void setConstant(float constant) { this->constant = (constant > 0) ? constant : 1.0; }
    void setLinear(float linear) { this->linear = (linear >= 0) ? linear : 1.0; }
    void setQuadratic(float quadratic) { this->quadratic = (quadratic >= 0) ? quadratic : 1.0; }
    
    float getCutOff() { return cutOff; }
    float getOuterCutOff() { return outerCutOff; }
    void setCutOff(float cutOff);
    void setOuterCutOff(float cutOff);
private:
    glm::vec3 direction;
    glm::vec3 position;
    float constant;
    float linear;
    float quadratic;
    float cutOff;     //stores angle (need to pass cos(cutOff) to shader)
    float outerCutOff;// same as cutOff
};

#endif