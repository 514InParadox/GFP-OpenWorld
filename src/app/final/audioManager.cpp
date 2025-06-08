#include "audioManager.hpp"

#include <iostream>         // For error output
#include <algorithm>        // For std::min, std::max
#include <glm/gtx/norm.hpp> // For glm::length2, if you prefer squared distance for performance

// Helper function to get asset full path. You might need to adjust this

std::string getAssetFullPath(const std::string &assetName)
{
    return "audio/" + assetName;
}

// 计算两个glm::vec2之间的距离
float calculateDistance(const glm::vec2 &lhs, const glm::vec2 &rhs)
{
    return glm::length(lhs - rhs);
}
AudioManager::AudioManager()
    : _audioEngine(nullptr), // 先初始化为 nullptr
      _footstepVolumeAccumulator(0.0f),
      _footstepVolumeFadeInTime(0.5f), // 0.5 秒达到最大音量
      _walkFootstepInterval(0.42f),     // 走路脚步声间隔 0.42 秒 (每步间隔)
      _runFootstepInterval(0.28f),     // 跑步脚步声间隔 0.28 秒 (每步间隔)
      _currentFootstepTimer(0.0f),     // 脚步声计时器
      _entityPatrolSound(nullptr),
      _entityChaseSound(nullptr),
      _entitySoundMaxDist(50.0f), // 怪物的声音在 50 单位距离内可听到
      _entitySoundMinDist(5.0f)   // 在 5 单位距离内音量最大
{
    _audioEngine = irrklang::createIrrKlangDevice();
    if (!_audioEngine)
    {
        std::cerr << "Error: Could not create irrKlang sound engine!" << std::endl;
        // Consider throwing an exception or setting a flag here
        return;
    }
}

AudioManager::~AudioManager()
{
    // 释放所有音效
    if (_walkFootstepSound)
        _walkFootstepSound->drop();
    if (_runFootstepSound)
        _runFootstepSound->drop();
    if (_entityPatrolSound)
        _entityPatrolSound->drop();
    if (_entityChaseSound)
        _entityChaseSound->drop();

    // 释放音频引擎
    if (_audioEngine)
        _audioEngine->drop();
}

void AudioManager::reset()
{
    if (_walkFootstepSound != nullptr)
        _walkFootstepSound->drop();
    if (_runFootstepSound != nullptr)
        _runFootstepSound->drop();
    if (_entityPatrolSound != nullptr)
        _entityPatrolSound->drop();
    if (_entityChaseSound != nullptr)
        _entityChaseSound->drop();
    if (_audioEngine != nullptr)
        _audioEngine->drop();

    _audioEngine = irrklang::createIrrKlangDevice();
    if (_audioEngine == nullptr)
    {
        std::cerr << "Error: Could not create irrKlang sound engine!" << std::endl;
        // Consider throwing an exception or setting a flag here
        return;
    }
    _footstepVolumeAccumulator = 0.0f;
    _footstepVolumeFadeInTime = 0.5f; // 0.5 秒达到最大音量
    _walkFootstepInterval = 0.44f;     // 走路脚步声间隔 0.4 秒 (每步间隔)
    _runFootstepInterval = 0.32f;     // 跑步脚步声间隔 0.25 秒 (每步间隔)
    _currentFootstepTimer = 0.0f;     // 脚步声计时器
    _entityPatrolSound = nullptr;
    _entityChaseSound = nullptr;
    _entitySoundMaxDist = 50.0f; // 怪物的声音在 50 单位距离内可听到
    _entitySoundMinDist = 5.0f;  // 在 5 单位距离内音量最大
}

void AudioManager::init()
{
    // 设置全局音量
    _audioEngine->setSoundVolume(0.8f);

    // 预加载并初始化实体音效
    _entityPatrolSound = _audioEngine->play3D(getAssetFullPath("entity_patrol.mp3").c_str(),
                                              irrklang::vec3df(0, 0, 0), // 位置将在更新时设置
                                              true, true, true);         // 循环, 暂停, 追踪
    if (_entityPatrolSound)
        _entityPatrolSound->setVolume(0.0f); // 初始静音

    _entityChaseSound = _audioEngine->play3D(getAssetFullPath("entity_chase.mp3").c_str(),
                                             irrklang::vec3df(0, 0, 0), // 位置将在更新时设置
                                             true, true, true);         // 循环, 暂停, 追踪
    if (_entityChaseSound)
        _entityChaseSound->setVolume(0.0f); // 初始静音
}

void AudioManager::updateListenerPosition(const glm::vec3 &cameraPosition,
                                          const glm::vec3 &cameraFront,
                                          const glm::vec3 &cameraUp)
{
    if (!_audioEngine)
        return;

    _audioEngine->setListenerPosition(
        irrklang::vec3df(cameraPosition.x, cameraPosition.y, cameraPosition.z),
        irrklang::vec3df(cameraFront.x, cameraFront.y, cameraFront.z),
        irrklang::vec3df(0, 0, 0), // 简化处理，速度设为0。根据相机位移计算
        irrklang::vec3df(cameraUp.x, cameraUp.y, cameraUp.z));
    // std::cerr   << "AudioManager: update listener position to " 
    //             << cameraPosition.x << " " << cameraPosition.y << " " << cameraPosition.z << std::endl;
}

void AudioManager::updatePlayerFootsteps(const glm::vec2 &playerCurrentPos,
                                         const glm::vec2 &playerPreviousPos,
                                         bool isPlayerRunning,
                                         float volume_in,
                                         float deltaTime)
{
    if (!_audioEngine)
        return;

    // std::cerr << "Player position varied by " << calculateDistance(playerCurrentPos, playerPreviousPos) << " units, "; 
    bool playerMoved = calculateDistance(playerCurrentPos, playerPreviousPos) > 0.001f;

    if (playerMoved)
    {
        // std::cerr << "is moving" << std::endl;
        // _footstepVolumeAccumulator = std::min(_footstepVolumeAccumulator + deltaTime, _footstepVolumeFadeInTime);
        // float currentFootstepVolume = _footstepVolumeAccumulator / _footstepVolumeFadeInTime;
        float currentFootstepVolume = volume_in;
        _currentFootstepTimer += deltaTime;
        float requiredInterval = isPlayerRunning ? _runFootstepInterval : _walkFootstepInterval;

        if (_currentFootstepTimer >= requiredInterval)
        {
            // 播放单次脚步声
            const char *soundToPlay = isPlayerRunning ? "pl_run.wav" : "pl_walk.wav";
            irrklang::ISound *footstep = _audioEngine->play2D(getAssetFullPath(soundToPlay).c_str(),
                                                              false, false, true); // 不循环, 不暂停, 追踪
            if (footstep)
            {
                footstep->setVolume(currentFootstepVolume); // 设置音量
                footstep->drop();                           // 播放后立即丢弃，让irrKlang管理其生命周期
            }
            _currentFootstepTimer = 0.0f; // 重置计时器
        }
    }
    else
    {
        // 玩家未移动，暂停脚步声并重置淡入累积器
        // if (_walkFootstepSound) _walkFootstepSound->setIsPaused(true);
        // if (_runFootstepSound) _runFootstepSound->setIsPaused(true);
        _footstepVolumeAccumulator = 0.0f;
    }
}

void AudioManager::updateEntitySound(const EntityLogic &entityLogic,
                                     const glm::vec2 &playerPosition)
{
    if (!_audioEngine || (!_entityPatrolSound && !_entityChaseSound))
        return;

    float distToEntity = calculateDistance(playerPosition, entityLogic.getEntityPos());
    irrklang::vec3df entitySoundPos(entityLogic.getEntityPos().x, 0.0f, entityLogic.getEntityPos().y); // 假设实体在y=0平面

    // 默认暂停所有实体音效
    if (_entityPatrolSound)
        _entityPatrolSound->setIsPaused(true);
    if (_entityChaseSound)
        _entityChaseSound->setIsPaused(true);

    if (distToEntity < _entitySoundMaxDist)
    {
        float volume = 1.0f; // 默认最大音量
        if (distToEntity > _entitySoundMinDist)
        {
            // 线性衰减：从 MIN_DIST 到 MAX_DIST 音量从 1.0 降到 0.0
            volume = 1.0f - ((distToEntity - _entitySoundMinDist) / (_entitySoundMaxDist - _entitySoundMinDist));
            volume = std::max(0.0f, std::min(1.0f, volume)); // 确保音量在 0 到 1 之间
        }

        if (entityLogic.Status == EntityStatus::CHASE)
        {
            if (_entityChaseSound)
            {
                _entityChaseSound->setVolume(volume);
                _entityChaseSound->setPosition(entitySoundPos);
                _entityChaseSound->setIsPaused(false);
            }
        }
        else
        { // EntityStatus::PATROL
            if (_entityPatrolSound)
            {
                _entityPatrolSound->setVolume(volume);
                _entityPatrolSound->setPosition(entitySoundPos);
                _entityPatrolSound->setIsPaused(false);
            }
        }
    }
}