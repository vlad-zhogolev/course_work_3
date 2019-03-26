#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light
{
public:
    Light();
    void setColor(glm::vec3 color) { this->color = color; }
    glm::vec3 getColor() { return color; }      
    void setAmbient(glm::vec3 ambient) { this->ambient = ambient; }
    void setDiffuse(glm::vec3 diffuse) { this->diffuse = diffuse; }
    void setSpecular(glm::vec3 specular) { this->specular = specular; }
    glm::vec3 getAmbient() { return ambient; }
    glm::vec3 getDiffuse() { return diffuse; }
    glm::vec3 getSpecular() { return specular; }
   
protected:    
    glm::vec3 color;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
};


#endif