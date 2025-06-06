#include "dialog.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>

// 读入文件时，读入的是 assetPath + std::to_string(i) + "-" + std::to_string(j) + ".obj"
// 将其读入之后，存储在 preLoad 中，准备对话时一句句输出到 dialogBox 中。
Dialog::Dialog(const std::string &assetPath, const DialogConfig& config) 
    : _config(config), _currentDialogTime(0.0f), _currentDialogIndex(0), 
      _started(false), _paused(false), _totalDialogGroups(0) {
    
    // 扫描对话文件，按照命名规则 i-j.obj 加载
    int dialogGroupIndex = 0;
    
    while (true) {
        std::vector<std::unique_ptr<Text>> dialogGroup;
        int textIndex = 0;
        bool foundAnyInGroup = false;
        
        // 尝试加载当前对话组的所有文本片段
        while (true) {
            std::string filename = assetPath + std::to_string(dialogGroupIndex) + "-" + std::to_string(textIndex) + ".obj";
            
            // 检查文件是否存在
            if (std::filesystem::exists(filename)) {
                try {
                    // 创建AdvancedModel并包装为Text
                    auto model = std::make_unique<AdvancedModel>(filename);
                    auto text = std::make_unique<Text>(std::move(model));
                    dialogGroup.push_back(std::move(text));
                    
                    foundAnyInGroup = true;
                    std::cout << "Loaded dialog file: " << filename << std::endl;
                } catch (const std::exception& e) {
                    std::cerr << "Failed to load dialog file " << filename << ": " << e.what() << std::endl;
                }
                textIndex++;
            } else {
                // 当前组没有更多文本了
                break;
            }
        }
        
        // 如果这个对话组找到了文本，添加到preLoad中
        if (foundAnyInGroup) {
            preLoad.push_back(std::move(dialogGroup));
              // 为每个对话组设置默认持续时间（可以根据需要调整）
            // 基础时间 + 文本数量 * 额外时间
            float groupTime = _config.baseDisplayTime + (textIndex * _config.timePerText);
            continueTime.push_back(groupTime);
            
            std::cout << "Loaded dialog group " << dialogGroupIndex 
                      << " with " << textIndex << " texts, duration: " << groupTime << "s" << std::endl;
            
            dialogGroupIndex++;
        } else {
            // 没有找到这个索引的对话组，停止搜索
            break;        }
    }
    
    _totalDialogGroups = preLoad.size();
    std::cout << "Dialog system initialized with " << _totalDialogGroups << " dialog groups" << std::endl;
}

void Dialog::proceed(const float &deltaTime) {
    // 如果暂停了，不处理任何逻辑
    if (_paused) {
        return;
    }
    
    // 如果还没开始对话，启动第一个对话组
    if (!_started && !preLoad.empty()) {
        _started = true;
        nextDialog();
    }
    
    // 更新当前对话的计时
    if (!dialogBox.empty() && _config.autoAdvance) {
        _currentDialogTime += deltaTime;
        
        // 检查当前对话是否应该结束
        if (shouldFinishCurrentDialog()) {
            dropCurrentDialog();
            if (!nextDialog() && _config.loopDialogs) {
                // 如果启用循环且到达末尾，重新开始
                resetDialog();
            }
        }
    }
    
    // 更新所有文本的生命周期
    updateTextLifetimes(deltaTime);
}

bool Dialog::isFinished() const {
    return _started && preLoad.empty() && dialogBox.empty() && dropText.empty();
}

bool Dialog::nextDialog() {
    // 如果没有更多预加载的对话，返回false
    if (_currentDialogIndex >= preLoad.size()) {
        return false;
    }
    
    // 将下一组对话从preLoad移动到dialogBox
    dialogBox = std::move(preLoad[_currentDialogIndex]);
      // 为新对话中的每个文本设置生命周期
    float currentGroupTime = continueTime[_currentDialogIndex];
    for (auto& text : dialogBox) {
        text->setLifeTime(currentGroupTime + _config.fadeOutTime); // 使用配置的淡出时间
    }
    
    // 重置当前对话计时器
    _currentDialogTime = 0.0f;
    _currentDialogIndex++;
    
    std::cout << "Started dialog group " << (_currentDialogIndex - 1) 
              << ", duration: " << currentGroupTime << "s" << std::endl;
    
    return true;
}

bool Dialog::shouldFinishCurrentDialog() const {
    if (_currentDialogIndex == 0) return false;
    
    float currentGroupTime = continueTime[_currentDialogIndex - 1];
    return _currentDialogTime >= currentGroupTime;
}

void Dialog::dropCurrentDialog() {
    // 将当前对话框中的文本移动到dropText中
    for (auto& text : dialogBox) {
        dropText.push_back(std::move(text));
    }
    dialogBox.clear();
    
    std::cout << "Dropped current dialog to fade out" << std::endl;
}

void Dialog::updateTextLifetimes(const float &deltaTime) {
    // Update dropText中文本的生命周期，移除过期的文本
    auto it = dropText.begin();
    while (it != dropText.end()) {
        if (!(*it)->Life(deltaTime)) {
            // 文本生命周期结束，移除它
            it = dropText.erase(it);
        } else {
            ++it;
        }
    }
}

void Dialog::drawDialogBox() const {
    // 绘制当前对话框中的所有文本
    for (const auto& text : dialogBox) {
        if (text) {
            text->draw();
        }
    }
}

void Dialog::drawDropText() const {
    // 绘制正在淡出的文本
    for (const auto& text : dropText) {
        if (text) {
            text->draw();
        }
    }
}

size_t Dialog::getCurrentDialogIndex() const {
    return _currentDialogIndex;
}

size_t Dialog::getTotalDialogCount() const {
    return preLoad.size() + (_currentDialogIndex > 0 ? 1 : 0);
}

float Dialog::getCurrentDialogProgress() const {
    if (_currentDialogIndex == 0 || continueTime.empty()) {
        return 0.0f;
    }
    
    float currentGroupTime = continueTime[_currentDialogIndex - 1];
    return std::min(1.0f, _currentDialogTime / currentGroupTime);
}

void Dialog::forceNextDialog() {
    if (!dialogBox.empty()) {
        dropCurrentDialog();
    }
    
    if (!nextDialog() && _config.loopDialogs) {
        resetDialog();
    }
}

void Dialog::resetDialog() {
    // 重置所有状态
    _currentDialogIndex = 0;
    _currentDialogTime = 0.0f;
    _started = false;
    _paused = false;
    
    // 清空当前对话和淡出文本
    dialogBox.clear();
    dropText.clear();
    
    // 重新加载对话（如果需要的话，这里假设preLoad已经被移动，需要重新构建）
    // 注意：这种实现假设对话数据仍然可用，实际实现可能需要重新加载文件
    
    std::cout << "Dialog system reset" << std::endl;
}

void Dialog::pauseDialog() {
    _paused = true;
    std::cout << "Dialog paused" << std::endl;
}

void Dialog::resumeDialog() {
    _paused = false;
    std::cout << "Dialog resumed" << std::endl;
}

void Dialog::setConfig(const DialogConfig& config) {
    _config = config;
}

const DialogConfig& Dialog::getConfig() const {
    return _config;
}