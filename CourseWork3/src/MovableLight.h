#ifndef MOVABLE_LIGHT_H
#define MOVABLE_LIGHT_H

#include "Light.h"

enum class Direction
{
    UPWARD,
    DOWNWARD,
    FORWARD,
    BACKWARD,
    LEFTWARD,    
    RIGHTWARD
};

const float movementSpeed = 2.5f;
const glm::vec3 LEFT  = glm::vec3(1, 0, 0);
const glm::vec3 UP    = glm::vec3(0, 1, 0);
const glm::vec3 FRONT = glm::vec3(0, 0, 1);

class MovableLight : public Light
{   
public:
    static bool pointLightsAreProcessed;

public:
    void processKeyboard(Direction, float deltaTime);
protected:
    glm::vec3 position;
};

#endif 