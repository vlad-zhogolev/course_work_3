#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Light
{
public:
    Light() = default;

    Light(glm::vec3 color)
        :_color(color)
        {}

    void setColor(glm::vec3 color) { _color = color; }     

    glm::vec3 getColor() { return _color; } 
   
protected:    
    glm::vec3 _color;
};


#endif