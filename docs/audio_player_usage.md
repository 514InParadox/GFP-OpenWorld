# AudioPlayer 使用指南

## 概述

AudioPlayer是一个基于irrKlang的音频播放器类，提供了简单易用的接口来加载和播放音频文件。

## 特性

- ✅ **音频文件加载**：支持WAV、MP3、OGG等格式
- ✅ **播放控制**：支持循环播放和单次播放
- ✅ **音量控制**：独立音量控制和主音量控制
- ✅ **状态检查**：检查音频是否正在播放
- ✅ **资源管理**：自动清理音频资源

## 基本使用

### 1. 创建和初始化

```cpp
#include "utils/audio_player.hpp"

// 创建AudioPlayer实例
AudioPlayer audioPlayer;

// 初始化音频引擎
if (!audioPlayer.initialize()) {
    std::cerr << "Failed to initialize audio player!" << std::endl;
    return;
}
```

### 2. 加载音频文件

```cpp
// 加载不同的音频文件
audioPlayer.loadAudio("walking", "audio/walking-sound.wav");
audioPlayer.loadAudio("background", "audio/background-music.mp3");
audioPlayer.loadAudio("effect", "audio/sound-effect.ogg");

// 检查加载的音频数量
std::cout << "Loaded " << audioPlayer.getLoadedAudioCount() << " audio files" << std::endl;
```

### 3. 播放音频

```cpp
// 播放音频（不循环，默认音量）
audioPlayer.playAudio("effect");

// 播放音频（循环播放，自定义音量）
audioPlayer.playAudio("background", true, 0.7f);

// 播放音频（不循环，低音量）
audioPlayer.playAudio("walking", false, 0.5f);
```

### 4. 控制播放

```cpp
// 检查是否正在播放
if (audioPlayer.isPlaying("background")) {
    std::cout << "Background music is playing" << std::endl;
}

// 停止特定音频
audioPlayer.stopAudio("background");

// 停止所有音频
audioPlayer.stopAllAudio();
```

### 5. 音量控制

```cpp
// 设置特定音频的音量
audioPlayer.setVolume("background", 0.8f);

// 设置主音量（影响所有音频）
audioPlayer.setMasterVolume(0.6f);
```

## 完整示例

```cpp
#include "utils/audio_player.hpp"
#include <iostream>

class GameAudioManager {
private:
    AudioPlayer _audioPlayer;
    
public:
    bool initialize() {
        // 初始化音频播放器
        if (!_audioPlayer.initialize()) {
            return false;
        }
        
        // 加载游戏音频
        _audioPlayer.loadAudio("bgm", "audio/background.mp3");
        _audioPlayer.loadAudio("footsteps", "audio/walking.wav");
        _audioPlayer.loadAudio("jump", "audio/jump.wav");
        _audioPlayer.loadAudio("pickup", "audio/pickup.wav");
        
        return true;
    }
    
    void startBackgroundMusic() {
        if (!_audioPlayer.isPlaying("bgm")) {
            _audioPlayer.playAudio("bgm", true, 0.6f); // 循环播放，60%音量
        }
    }
    
    void startWalking() {
        if (!_audioPlayer.isPlaying("footsteps")) {
            _audioPlayer.playAudio("footsteps", true, 0.8f); // 循环播放
        }
    }
    
    void stopWalking() {
        _audioPlayer.stopAudio("footsteps");
    }
    
    void playJumpSound() {
        _audioPlayer.playAudio("jump", false, 1.0f); // 单次播放
    }
    
    void playPickupSound() {
        _audioPlayer.playAudio("pickup", false, 0.9f); // 单次播放
    }
    
    void setMasterVolume(float volume) {
        _audioPlayer.setMasterVolume(volume);
    }
};
```

## API 参考

### 构造函数和析构函数

```cpp
AudioPlayer();          // 构造函数
~AudioPlayer();         // 析构函数（自动清理资源）
```

### 初始化

```cpp
bool initialize();      // 初始化音频引擎，返回是否成功
bool isInitialized();   // 检查是否已初始化
```

### 音频加载

```cpp
bool loadAudio(const std::string& name, const std::string& filePath);
// 参数：
//   name - 音频标识符
//   filePath - 音频文件路径
// 返回：是否加载成功
```

### 播放控制

```cpp
bool playAudio(const std::string& name, bool loop = false, float volume = 1.0f);
// 参数：
//   name - 音频标识符
//   loop - 是否循环播放（默认：false）
//   volume - 音量（0.0-1.0，默认：1.0）
// 返回：是否开始播放成功

void stopAudio(const std::string& name);     // 停止特定音频
void stopAllAudio();                         // 停止所有音频
bool isPlaying(const std::string& name);     // 检查是否正在播放
```

### 音量控制

```cpp
void setVolume(const std::string& name, float volume);  // 设置特定音频音量
void setMasterVolume(float volume);                     // 设置主音量
```

### 信息查询

```cpp
size_t getLoadedAudioCount();  // 获取已加载音频文件数量
```

## 支持的音频格式

- **WAV** - 无压缩音频，适合短音效
- **MP3** - 压缩音频，适合背景音乐
- **OGG** - 开源压缩格式
- **FLAC** - 无损压缩格式

## 最佳实践

### 1. 音频文件组织

```
audio/
├── music/
│   ├── background.mp3
│   ├── menu.mp3
│   └── victory.mp3
├── effects/
│   ├── jump.wav
│   ├── pickup.wav
│   └── explosion.wav
└── ambient/
    ├── wind.ogg
    └── water.ogg
```

### 2. 音频管理策略

- **背景音乐**：使用MP3格式，循环播放，较低音量
- **音效**：使用WAV格式，单次播放，适中音量
- **环境音**：使用OGG格式，循环播放，很低音量

### 3. 性能优化

- 在游戏开始时预加载所有音频文件
- 使用合适的音频格式和质量
- 及时停止不需要的循环音频
- 合理设置音量避免音频失真

### 4. 错误处理

```cpp
// 检查初始化
if (!audioPlayer.initialize()) {
    // 处理初始化失败
    std::cerr << "Audio initialization failed" << std::endl;
    // 可以选择禁用音频功能继续运行
}

// 检查文件加载
if (!audioPlayer.loadAudio("bgm", "audio/background.mp3")) {
    std::cerr << "Failed to load background music" << std::endl;
    // 可以加载备用音频或跳过
}
```

## 测试应用程序

运行`AudioPlayerTestApp`来测试AudioPlayer的功能：

### 控制说明
- **1** - 播放步行音效（循环）
- **2** - 播放巡逻音乐（循环）
- **3** - 播放追逐音乐（单次）
- **SPACE** - 停止所有音频
- **S** - 停止步行音效
- **+/-** - 调整音量
- **ESC** - 退出

---

*AudioPlayer基于irrKlang实现，提供了简单易用的音频播放接口。* 