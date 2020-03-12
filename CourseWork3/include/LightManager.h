#ifndef LIGHT_MANAGER_H
#define LIGHT_MANAGER_H

#include <vector>
#include <Aliases.h>
#include <Lights/PointLight.h>
#include <Lights/SpotLight.h>s
#include <GLFW/glfw3.h>

enum class ActiveLightType 
{
    NONE,
    POINT,
    SPOT
};

enum class Direction
{
    UP,
    DOWN,
    FRONT,
    BACK,
    LEFT,
    RIGHT
};

class LightManager 
{ 
    static float movementSpeed;
    static const glm::vec3 LEFT;
    static const glm::vec3 UP;
    static const glm::vec3 FRONT;

public:
    LightManager(PointLights& pointLights, SpotLights& spotLights) : pointLights(pointLights), spotLights(spotLights) {};    
    void switchToNext();
    void switchToPrevious();
    void switchLightType(ActiveLightType type);
    void translateCurrentLight(Direction dir);
    void setActiveLightType(ActiveLightType type) { this->activeType = type; }
    void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
    void updateDeltaTime(float deltaTime) { this->deltaTime = deltaTime >= 0 ? deltaTime : 0; }
private:    
    PointLights& pointLights;
    SpotLights& spotLights;
    int curPointLight = 0;
    int curSpotLight = 0;
    ActiveLightType activeType = ActiveLightType::NONE;
    float deltaTime = 0;
};

#endif // !LIGHT_MANAGER_H
