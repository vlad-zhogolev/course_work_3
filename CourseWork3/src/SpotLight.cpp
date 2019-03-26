#include "SpotLight.h"

using namespace std;

SpotLight::SpotLight(glm::vec3 position, glm::vec3 color, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular, float constant, float linear, float quadratic, float cutOff, float outerCutOff)
{
    this->position = position;
    this->color = color;
    this->direction = direction;
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
    this->cutOff = cutOff;
    this->outerCutOff = outerCutOff;
}

void SpotLight::setCutOff(float cutOff)
{
    if (cutOff < 0)
        this->cutOff = 0;
    else if (cutOff > 90)
        this->cutOff = 90;
    else
        this->cutOff = cutOff;
}

void SpotLight::setOuterCutOff(float outerCutOff)
{
    if (outerCutOff < 0)
        this->outerCutOff = 0;
    else if (outerCutOff > 90)
        this->outerCutOff = 90;
    else
        this->outerCutOff = outerCutOff;
}