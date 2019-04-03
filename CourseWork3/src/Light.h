#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light
{
public:
    Light() = default;

    Light(glm::vec3 color, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular):
        _color(color), 
        _ambient(ambient),
        _diffuse(diffuse),
        _specular(specular){}

    void setColor(glm::vec3 color) { _color = color; }     
    void setAmbient(glm::vec3 ambient) { _ambient = ambient; }
    void setDiffuse(glm::vec3 diffuse) { _diffuse = diffuse; }
    void setSpecular(glm::vec3 specular) { _specular = specular; }

    glm::vec3 getColor() { return _color; } 
    glm::vec3 getAmbient() { return _ambient; }
    glm::vec3 getDiffuse() { return _diffuse; }
    glm::vec3 getSpecular() { return _specular; }
   
protected:    
    glm::vec3 _color;
    glm::vec3 _ambient;
    glm::vec3 _diffuse;
    glm::vec3 _specular;
};


#endif