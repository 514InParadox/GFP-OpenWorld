#pragma once

#include "text.hpp"
#include <string>
#include <vector>
#include <memory>
#include <filesystem>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class GLSLProgram;  // forward declaration for shader

// Dialog configuration structure
struct DialogConfig {
    float baseDisplayTime = 2.0f;        // Base time for each dialog group
    float timePerText = 1.0f;             // Additional time per text fragment
    float fadeOutTime = 2.0f;             // Extra time for fade out effect
    float lineSpacing = 0.4f;             // Vertical spacing between lines
    bool autoAdvance = true;              // Whether to auto-advance dialogs
    bool loopDialogs = false;             // Whether to loop through dialogs
    float charSpacing = 0.1f;             // Horizontal spacing between the start of one char to the start of the next
    float lineMaxWidth = 1.0f;            // Maximum width of a line before wrapping
    glm::vec3 textScale = glm::vec3(0.2f); // Uniform scale for all text models
};

// 维护对话
class Dialog {
public:
    Dialog(const std::string &assetPath, const DialogConfig& config = DialogConfig{});

    void proceed(const float &deltaTime);

    
    // Check if all dialogs have been displayed
    bool isFinished() const;

    
    // Draw current dialog texts
    void drawDialogBox(GLSLProgram* shader) const;
    
    // Draw fading out texts
    void drawDropText(GLSLProgram* shader) const;

    // Update and draw all dialogs
    void draw(const float &deltaTime, GLSLProgram* shader);
    void draw(const float &deltaTime, GLSLProgram* shader, const glm::vec3& cameraPos, const glm::quat& cameraRot);
    
    // Get current dialog info
    // size_t getCurrentDialogIndex() const; // Removed
    size_t getTotalDialogCount() const;
    // float getCurrentDialogProgress() const; // Removed
    
    // Manual control
    void forceNextDialog();
    void resetDialog();
    void pauseDialog();
    void resumeDialog();
    
    // Configuration
    void setConfig(const DialogConfig& config);
    const DialogConfig& getConfig() const;

    // 设置文本显示的基准位置
    void setBasePosition(const glm::vec3& pos);

    // 获取当前基准位置
    glm::vec3 getBasePosition() const;
    
private:
    // bool nextDialog(); // Removed
    // bool shouldFinishCurrentDialog() const; // Removed
    
    // Move texts from dialogBox to dropText for cleanup
    void dropCurrentDialog();
    
    // Update text lifetimes and remove expired texts
    void updateTextLifetimes(const float &deltaTime);

    // std::vector<std::vector<std::unique_ptr<Text>>> preLoad; // Removed
    // std::vector<float> continueTime; // Removed

    // 在对话框内的当前显示文本
    std::vector<std::unique_ptr<Text>> dialogBox;
    
    // 在对话框外准备销毁的文本
    std::vector<std::unique_ptr<Text>> dropText;
    
    // float _currentDialogTime; // Removed
    // size_t _currentDialogIndex; // Removed
    
    // 对话是否已开始
    bool _started;
    
    // 对话是否暂停
    bool _paused;
    
    // 总对话组数量（用于循环等功能）
    size_t _totalDialogGroups;
    
    // 配置
    DialogConfig _config;

    // 文本显示的基准位置
    glm::vec3 _basePosition{0.0f, 0.0f, 0.0f};

    // 保存对话文件列表，便于 reset 重新加载
    std::vector<std::string> _dialogFiles;
};