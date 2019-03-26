#ifndef SCENE_LOADER_H
#define SCENE_LOADER_H

#include <iostream>
#include <vector>
#include "DirectionalLight.h"
#include "PointLight.h"
#include "SpotLight.h"
#include "Model.h"
#include "Object.h"
#include <fstream>
#include <sstream>

class SceneLoader
{
public:
    SceneLoader();
    void loadScene(std::string lightsDataPath, std::string modelsDataPath, std::vector<DirectionalLight>& dirLights, std::vector<PointLight>& pointLights, std::vector<SpotLight>& spotLights, std::vector<Model>& models, vector<Object>& objects);
private:
    DirectionalLight loadDirectionalLight(std::stringstream& lightData, bool& good);
    PointLight loadPointLight(std::stringstream& lightData, bool& good);
    SpotLight loadSpotLight(std::stringstream& lightData, bool& good);
    int getModelIndex(std::string path, std::vector<Model>& modles);
    glm::vec3 getVec3(std::stringstream& data);   
    bool checkRangeVec3(glm::vec3 vector, double left, double right, string message);
    bool checkAttenuation(double constant, double linear, double quadratic);
    bool checkAngles(double cutOff, double outerCutOff);
    bool checkScale(glm::vec3 scale);
    void exitOnError();
};

#endif

