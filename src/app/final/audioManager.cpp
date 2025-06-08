#include "audioManager.hpp"

#include <iostream>         // For error output
#include <iomanip>          // For std::setprecision
#include <algorithm>        // For std::min, std::max
#include <glm/gtx/norm.hpp> // For glm::length2, if you prefer squared distance for performance

// Helper function to get asset full path for audio files
std::string getAudioAssetPath(const std::string &assetName)
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
      _entitySoundMinDist(0.0f)   // 在 0 单位距离时音量最大
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
    _entitySoundMinDist = 0.0f;  // 在 0 单位距离时音量最大
}

void AudioManager::init()
{
    // 设置全局音量
    _audioEngine->setSoundVolume(0.8f);

    // 预加载并初始化实体音效
    _entityPatrolSound = _audioEngine->play2D(getAudioAssetPath("entity_patrol.wav").c_str(),
                                              true, true, true);         // 循环, 暂停开始, 追踪
    if (_entityPatrolSound)
        _entityPatrolSound->setVolume(1.0f); // 设置为最大音量，但开始时暂停

    _entityChaseSound = _audioEngine->play2D(getAudioAssetPath("entity_chase.wav").c_str(),
                                             true, true, true);         // 循环, 暂停开始, 追踪
    if (_entityChaseSound)
        _entityChaseSound->setVolume(1.0f); // 设置为最大音量，但开始时暂停

    // 预加载脚步声音效以避免首次播放时的卡顿
    std::cout << "Preloading footstep sounds..." << std::endl;
    
    // 方法2：静音播放预加载（强制加载音频到缓存）
    irrklang::ISound* preloadWalk = _audioEngine->play2D(getAudioAssetPath("pl_walk.wav").c_str(),
                                                         false, true, true); // 不循环, 暂停, 追踪
    if (preloadWalk) {
        preloadWalk->setVolume(0.0f); // 静音预加载
        std::cout << "Walk sound preloaded via silent playback" << std::endl;
        preloadWalk->drop(); // 立即释放，但音频已被缓存
    }
    
    irrklang::ISound* preloadRun = _audioEngine->play2D(getAudioAssetPath("pl_run.wav").c_str(),
                                                        false, true, true); // 不循环, 暂停, 追踪
    if (preloadRun) {
        preloadRun->setVolume(0.0f); // 静音预加载
        std::cout << "Run sound preloaded via silent playback" << std::endl;
        preloadRun->drop(); // 立即释放，但音频已被缓存
    }
    
    std::cout << "Footstep sounds preloaded successfully!" << std::endl;
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
            irrklang::ISound *footstep = _audioEngine->play2D(getAudioAssetPath(soundToPlay).c_str(),
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

    // 计算玩家与实体的距离
    float distToEntity = calculateDistance(playerPosition, entityLogic.getEntityPos());
    
    // 默认暂停所有实体音效
    if (_entityPatrolSound)
        _entityPatrolSound->setIsPaused(true);
    if (_entityChaseSound)
        _entityChaseSound->setIsPaused(true);
    
    // 根据实体状态播放相应音频
    if (entityLogic.Status == EntityStatus::PATROL)
    {
        // PATROL 状态：播放 patrol 音频
        if (_entityPatrolSound && distToEntity <= _entitySoundMaxDist) // 距离 <= 50
        {
            // 计算基于距离的音量衰减 (0到50平方衰减)
            float normalizedDistance = distToEntity / _entitySoundMaxDist; // 归一化到 [0, 1]
            float volume = 1.0f - (normalizedDistance * normalizedDistance); // 平方衰减
            volume = std::max(0.0f, std::min(1.0f, volume)); // 确保音量在 [0, 1] 范围内
            
            // 设置音量并确保音频正在播放
            _entityPatrolSound->setVolume(volume);
            _entityPatrolSound->setIsPaused(false);
            
            // Debug output
            static float patrolDebugTimer = 0.0f;
            patrolDebugTimer += 0.016f; // 假设60fps
            if (patrolDebugTimer >= 1.0f) // 每秒输出一次
            {
                std::cout << "PATROL Audio - Distance: " << std::fixed << std::setprecision(1) 
                          << distToEntity << ", Volume: " << std::setprecision(2) << volume << std::endl;
                patrolDebugTimer = 0.0f;
            }
        }
    }
    else if (entityLogic.Status == EntityStatus::CHASE)
    {
        // CHASE 状态：播放 chase 音频
        if (_entityChaseSound && distToEntity <= _entitySoundMaxDist) // 距离 <= 50
        {
            // 计算基于距离的音量衰减 (0到50平方衰减)
            float normalizedDistance = distToEntity / _entitySoundMaxDist; // 归一化到 [0, 1]
            float volume = 1.0f - (normalizedDistance * normalizedDistance); // 平方衰减
            volume = std::max(0.0f, std::min(1.0f, volume)); // 确保音量在 [0, 1] 范围内
            
            // 设置音量并确保音频正在播放
            _entityChaseSound->setVolume(volume);
            _entityChaseSound->setIsPaused(false);
            
            // Debug output
            static float chaseDebugTimer = 0.0f;
            chaseDebugTimer += 0.016f; // 假设60fps
            if (chaseDebugTimer >= 1.0f) // 每秒输出一次
            {
                std::cout << "CHASE Audio - Distance: " << std::fixed << std::setprecision(1) 
                          << distToEntity << ", Volume: " << std::setprecision(2) << volume << std::endl;
                chaseDebugTimer = 0.0f;
            }
        }
    }
    
    // Debug output for entity status changes
    static EntityStatus lastStatus = EntityStatus::PATROL;
    if (entityLogic.Status != lastStatus)
    {
        std::string statusStr = (entityLogic.Status == EntityStatus::CHASE) ? "CHASE" : 
                               (entityLogic.Status == EntityStatus::PATROL) ? "PATROL" : "OTHER";
        std::cout << "Entity Status Changed to: " << statusStr << std::endl;
        lastStatus = entityLogic.Status;
    }
}

void AudioManager::stopEntitySounds()
{
    // 停止并暂停所有实体音效
    if (_entityPatrolSound) {
        _entityPatrolSound->setIsPaused(true);
        _entityPatrolSound->setVolume(0.0f);
    }
    if (_entityChaseSound) {
        _entityChaseSound->setIsPaused(true);
        _entityChaseSound->setVolume(0.0f);
    }
    
    std::cout << "Entity sounds stopped." << std::endl;
}