#pragma once

#include <irrKlang/irrKlang.h>
#pragma comment(lib, "irrKlang.lib") // Link with irrKlang.dll

#include <glm/glm.hpp> // For glm::vec2, glm::vec3
#include <chrono>      // For time tracking
#include "entity.hpp"  // For EntityLogic and EntityStatus

// 确保在其他地方定义了 getAssetFullPath 函数，或者将其作为参数传入
// std::string getAssetFullPath(const std::string& assetName); 

class AudioManager {
public:
    // 构造函数：初始化音频引擎
    AudioManager();

    // 析构函数：释放音频资源
    ~AudioManager();

    // 加载所有音效
    void init();

    // 更新监听器（玩家相机）的位置和方向
    void updateListenerPosition(const glm::vec3& cameraPosition, 
                                const glm::vec3& cameraFront, 
                                const glm::vec3& cameraUp);

    void reset();
    // 更新玩家脚步声
    // @param  playerCurrentPos: 玩家当前位置
    // @param  playerPreviousPos: 玩家上一帧位置 (用于检测是否移动)
    // @param  isPlayerRunning: 玩家是否处于跑步状态
    // @param  volume_in: 外置传入的脚步声音量参数
    // @param  deltaTime: 帧时间
    void updatePlayerFootsteps(const glm::vec2& playerCurrentPos, 
                               const glm::vec2& playerPreviousPos, 
                               bool isPlayerRunning, 
                               float volume_in,
                               float deltaTime);

    // 更新实体（_entity）的音效
    // @param  entityLogic: 逻辑实体对象（包含其位置和状态）
    // @param  playerPosition: 玩家位置（用于计算距离衰减）
    void updateEntitySound(const EntityLogic& entityLogic, 
                           const glm::vec2& playerPosition);

private:
    irrklang::ISoundEngine* _audioEngine = nullptr; // irrKlang 音频引擎

    // 玩家脚步声
    irrklang::ISound* _walkFootstepSound = nullptr;
    irrklang::ISound* _runFootstepSound  = nullptr;
    float _footstepVolumeAccumulator;   // 脚步声音量累积器，用于淡入效果
    float _footstepVolumeFadeInTime;    // 脚步声淡入时间
    float _walkFootstepInterval;        // 走路脚步声播放间隔
    float _runFootstepInterval;         // 跑步脚步声播放间隔
    float _currentFootstepTimer;        // 当前脚步声计时器

    // 实体音效
    irrklang::ISound* _entityPatrolSound = nullptr;
    irrklang::ISound* _entityChaseSound = nullptr;
    float _entitySoundMaxDist;          // 实体音效最大可听距离
    float _entitySoundMinDist;          // 实体音效最大音量距离
};