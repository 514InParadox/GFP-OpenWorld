#include "srcDef.hpp"

// ===========================================================
// ||                     game constant                     ||
// ===========================================================

// Definition of non-const variable
float cameraRotateSpeed = 0.002f;

// constexpr std::pair<int, int> mitaCoord = std::make_pair(286, 188);

// ===========================================================
// ||                     resource path                     ||
// ===========================================================

// Definitions of all const std::string variables
const std::string entityPath = "resource/model/entity.obj";
const std::string mitaPath   = "resource/model/mita.obj";
const std::string mapPath    = "resource/model/map.obj";

const std::string texVertexShaderAddr   = "shader/vertex/oneTexture_diffuseLight.vert";
const std::string texFragmentShaderAddr = "shader/fragment/oneTexture_diffuseLight.frag";

const std::string vertexShaderAddr   = "shader/vertex/initSceneApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/initSceneApp.frag";

const std::string entityVertexShaderAddr   = "shader/vertex/entity.vert";
const std::string entityFragmentShaderAddr = "shader/fragment/entity.frag";

const std::string mapVertexShaderAddr   = "shader/vertex/map.vert";
const std::string mapFragmentShaderAddr = "shader/fragment/map.frag";

const std::string interfaceVertexShaderAddr = "shader/vertex/interface.vert";
const std::string interfaceFragmentShaderAddr = "shader/fragment/interface.frag";

// Definition of const std::vector
const std::vector<std::string> skyboxTexturePaths = {
    "resource/texture/skybox/default/right.jpg",
    "resource/texture/skybox/default/left.jpg",
    "resource/texture/skybox/default/top.jpg",
    "resource/texture/skybox/default/bottom.jpg",
    "resource/texture/skybox/default/front.jpg",
    "resource/texture/skybox/default/back.jpg"
};

const std::string startInterfaceImageAddr = "resource/texture/startScene.png";
const std::string loseInterfaceImageAddr  = "resource/texture/loseScene.png";
const std::string winInterfaceImageAddr   = "resource/texture/winScene.png";