#pragma once

#include <string>
#include <vector>
#include <math.h>

// ===========================================================
// ||                     game constant                     ||
// ===========================================================
// modifiable theoratically

constexpr float playerMoveSpeedSlow = 6.0f;
constexpr float playerMoveSpeedFast = 12.0f;

constexpr float entityMoveSpeedSlow = 3.0f;
constexpr float entityMoveSpeedFast = 11.0f;

// Non-const variables need external linkage declaration
extern float cameraRotateSpeed;

constexpr std::pair<int, int> mitaCoord = std::make_pair(286, 189);

constexpr float EntityTriggleDist = 1.0f;
constexpr float MitaTriggleDist = 5.0f;

// ===========================================================
// ||                 math & util constant                  ||
// ===========================================================

constexpr float pi = 3.14159265358979323846f;
constexpr float a2r = pi / 180;

// used for traverse neighboring lattices
constexpr int dx[] = {0, 1, -1, 0, 0};
constexpr int dy[] = {0, 0, 0, 1, -1};

// ===========================================================
// ||                     resource path                     ||
// ===========================================================

// External declarations for non-constexpr variables
extern const std::string entityPath;
extern const std::string mitaPath;
extern const std::string mapPath;
extern const std::string lightPath;

extern const std::string texVertexShaderAddr;
extern const std::string texFragmentShaderAddr;

extern const std::string vertexShaderAddr;
extern const std::string fragmentShaderAddr;

extern const std::string entityVertexShaderAddr;
extern const std::string entityFragmentShaderAddr;

extern const std::string mapVertexShaderAddr;
extern const std::string mapFragmentShaderAddr;

extern const std::string mitaVertexShaderAddr;
extern const std::string mitaFragmentShaderAddr;

extern const std::string emissiveVertexShaderAddr;
extern const std::string emissiveFragmentShaderAddr;

extern const std::string screenQuadVertexShaderAddr;
extern const std::string extractBrightShaderAddr;
extern const std::string blurShaderAddr;
extern const std::string combineShaderAddr;

extern const std::string interfaceVertexShaderAddr;
extern const std::string interfaceFragmentShaderAddr;

extern const std::vector<std::string> skyboxTexturePaths;

extern const std::string startInterfaceImageAddr;
extern const std::string loseInterfaceImageAddr;
extern const std::string winInterfaceImageAddr;
