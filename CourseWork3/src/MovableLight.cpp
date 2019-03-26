#include "MovableLight.h"

using namespace std;

void MovableLight::processKeyboard(Direction direction, float deltaTime)
{
    float velocity = movementSpeed * deltaTime;

    if (direction == Direction::FORWARD)
        position += FRONT * velocity;
    if (direction == Direction::BACKWARD)
        position -= FRONT * velocity;
    if (direction == Direction::LEFTWARD)
        position += LEFT * velocity;
    if (direction == Direction::RIGHTWARD)
        position -= LEFT * velocity;
    if (direction == Direction::UPWARD)
        position += UP * velocity;
    if (direction == Direction::DOWNWARD)
        position -= UP * velocity;
}