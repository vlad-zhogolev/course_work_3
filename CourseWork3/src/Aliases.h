#include "PointLight.h"
#include "SpotLight.h"
#include "DirectionalLight.h"
#include "Model.h"
#include "Object.h"

#include <vector>
#include <memory>

using DirectionalLights = vector<DirectionalLight>;
using PointLights = vector<PointLight>;
using SpotLights = vector<SpotLight>;
using Objects = vector<Object>;
using Models = vector<std::shared_ptr<Model>>;