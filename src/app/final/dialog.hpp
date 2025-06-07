#pragma once

#include "text.hpp"
#include "utils/glsl_program.hpp"
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

// Dialog configuration structure
struct DialogConfig {
    float baseDisplayTime = 2.0f;        // Base time for each dialog group
    float timePerText = 1.0f;             // Additional time per text fragment
    float fadeOutTime = 2.0f;             // Extra time for fade out effect
    bool autoAdvance = true;              // Whether to auto-advance dialogs
    bool loopDialogs = false;             // Whether to loop through dialogs
};

// 维护对话
class Dialog {
public:
    Dialog(const std::string &assetPath, const DialogConfig& config = DialogConfig{});

    void proceed(const float &deltaTime);

    void start();
    
    // Check if all dialogs have been displayed
    bool isFinished() const;

    void draw(const float &deltaTime, GLSLProgram &shader);
    
    // Draw current dialog texts
    //void drawDialogBox() const;
    void drawDialogBox(GLSLProgram &shader) const;
    
    // Draw fading out texts
    //void drawDropText() const;
    void drawDropText(GLSLProgram &shader) const;
    
    // Get current dialog info
    size_t getCurrentDialogIndex() const;
    size_t getTotalDialogCount() const;
    float getCurrentDialogProgress() const;
    
    // Manual control
    void forceNextDialog();
    void resetDialog();
    void pauseDialog();
    void resumeDialog();
    
    // Configuration
    void setConfig(const DialogConfig& config);
    const DialogConfig& getConfig() const;
    
private:
    // Move next dialog group from preLoad to dialogBox
    bool nextDialog();

    // Check if current dialog should finish based on time
    bool shouldFinishCurrentDialog() const;
    
    // Move texts from dialogBox to dropText for cleanup
    void dropCurrentDialog();
    
    // Update text lifetimes and remove expired texts
    void updateTextLifetimes(const float &deltaTime);

    // 预加载的对话组，每组包含一句对话的多个文本片段
    std::vector<std::vector<std::unique_ptr<Text>>> preLoad;

    // 每组对话的持续时间
    std::vector<float> continueTime;

    // 在对话框内的当前显示文本
    std::vector<std::unique_ptr<Text>> dialogBox;
    
    // 在对话框外准备销毁的文本
    std::vector<std::unique_ptr<Text>> dropText;
    
    // 当前对话的计时器
    float _currentDialogTime;
    
    // 当前对话组的索引
    size_t _currentDialogIndex;
    
    // 对话是否已开始
    bool _started;
    
    // 对话是否暂停
    bool _paused;
    
    // 总对话组数量（用于循环等功能）
    size_t _totalDialogGroups;
    
    // 配置
    DialogConfig _config;
};