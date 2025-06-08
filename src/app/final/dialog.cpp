#include "dialog.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>

// 按照 "Text_XXX.obj" 的顺序读取文件
// 读入后存入 preLoad，依次在对话框中显示
Dialog::Dialog(const std::string &assetPath, const DialogConfig& config) 
    : _config(config), _currentDialogTime(0.0f), _currentDialogIndex(0), 
      _started(false), _paused(false), _totalDialogGroups(0) {
    

    // 按照 Text_001.obj 等文件依次加载对话
    int textNumber = 1;
    while (true) {
        char nameBuf[16];
        std::snprintf(nameBuf, sizeof(nameBuf), "Text_%03d.obj", textNumber);
        std::string filename = assetPath + nameBuf;

        if (std::filesystem::exists(filename)) {
            try {
                auto model = std::make_unique<AdvancedModel>(filename);
                auto text = std::make_unique<Text>(std::move(model));
                // Rotate text upright so it's visible in first-person view
                text->getTransform().rotation =
                    glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                std::vector<std::unique_ptr<Text>> dialogGroup;
                dialogGroup.push_back(std::move(text));
                preLoad.push_back(std::move(dialogGroup));

                // 记录文件名以便 reset 重新加载
                _dialogFiles.push_back(filename);

                float groupTime = _config.baseDisplayTime + _config.timePerText;
                continueTime.push_back(groupTime);

                std::cout << "Loaded dialog file: " << filename << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Failed to load dialog file " << filename << ": " << e.what() << std::endl;
            }
            ++textNumber;
        } else {
            break;        
        }
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
    // 若没有成功加载任何对话，则视为已结束，避免外部循环进入死循环
    if (_totalDialogGroups == 0) {
        return true;
    }

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
        // 设置文本位置到基准位置
        text->getTransform().position = _basePosition;
        // 确保文本竖直显示
        text->getTransform().rotation =
            glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
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

void Dialog::drawDialogBox(GLSLProgram* shader) const {
    // 绘制当前对话框中的所有文本
    for (const auto& text : dialogBox) {
        if (text && shader) {
            shader->setUniformMat4("model", text->getModelMatrix());
            text->draw();
        }
    }
}

void Dialog::drawDropText(GLSLProgram* shader) const {
    // 绘制正在淡出的文本
    for (const auto& text : dropText) {
        if (text && shader) {
            shader->setUniformMat4("model", text->getModelMatrix());
            text->draw();
        }
    }
}

void Dialog::draw(const float &deltaTime, GLSLProgram* shader) {
    proceed(deltaTime);
    drawDialogBox(shader);
    drawDropText(shader);
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

    preLoad.clear();
    continueTime.clear();

    // 根据记录的文件重新加载文本
    for (const auto& file : _dialogFiles) {
        try {
            auto model = std::make_unique<AdvancedModel>(file);
            auto text = std::make_unique<Text>(std::move(model));
            text->getTransform().position = _basePosition;

            std::vector<std::unique_ptr<Text>> group;
            group.push_back(std::move(text));
            preLoad.push_back(std::move(group));

            float groupTime = _config.baseDisplayTime + _config.timePerText;
            continueTime.push_back(groupTime);
        } catch (const std::exception& e) {
            std::cerr << "Failed to reload dialog file " << file << ": " << e.what() << std::endl;
        }
    }

    _totalDialogGroups = preLoad.size();
    
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

void Dialog::setBasePosition(const glm::vec3& pos) {
    _basePosition = pos;

    // 更新当前所有文本的位置
    for (auto& text : dialogBox) {
        text->getTransform().position = _basePosition;
    }
    for (auto& text : dropText) {
        text->getTransform().position = _basePosition;
    }
}

glm::vec3 Dialog::getBasePosition() const {
    return _basePosition;
}