#include "dialog.hpp"
#include <iostream>
#include <filesystem>
#include <algorithm>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

// 按照 "Text_XXX.obj" 的顺序读取文件
// 读入后存入 preLoad，依次在对话框中显示
Dialog::Dialog(const std::string &assetPath, const DialogConfig& config)
    : _config(config), /*_currentDialogTime(0.0f), _currentDialogIndex(0),*/ // Removed members
      _started(true), _paused(false), _totalDialogGroups(0) {
    
    // New text layout logic
    float currentLineWidth = 0.0f;
    float currentLineYOffset = 0.0f; // Represents the Y offset for the current line from _basePosition

    int textNumber = 1;
    while (true) {
        char nameBuf[16];
        std::snprintf(nameBuf, sizeof(nameBuf), "Text_%03d.obj", textNumber);
        std::string filename = assetPath + nameBuf;

        if (std::filesystem::exists(filename)) {
            try {
                auto model = std::make_unique<AdvancedModel>(filename);
                auto text = std::make_unique<Text>(std::move(model));

                // Apply scale
                text->getTransform().scale = _config.textScale;

                // Apply base rotation (from Step 1)
                glm::quat baseRot =
                    glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                text->getTransform().rotation = baseRot;

                // Check for line wrapping
                // _config.charSpacing is used as the advance width for each character model
                if (currentLineWidth + _config.charSpacing > _config.lineMaxWidth && currentLineWidth > 0.0f) { // also check currentLineWidth > 0 to prevent wrapping if first char is too wide
                    currentLineYOffset += _config.lineSpacing; // Move to the next line
                    currentLineWidth = 0.0f;                   // Reset horizontal position for the new line
                }

                // Set position
                text->getTransform().position =
                    _basePosition + glm::vec3(0.0f, -currentLineYOffset, currentLineWidth);
                
                // Advance horizontal position for the next character
                currentLineWidth += _config.charSpacing;

                dialogBox.push_back(std::move(text));
                _dialogFiles.push_back(filename); // Keep track of loaded files for reset

                std::cout << "Loaded dialog file: " << filename << " at width: " << (currentLineWidth - _config.charSpacing) << ", height: " << -currentLineYOffset << std::endl;
            } catch (const std::exception& e) {
                std::cerr << "Failed to load dialog file " << filename << ": " << e.what() << std::endl;
            }
            ++textNumber;
        } else {
            break; // No more text files
        }
    }
    _totalDialogGroups = dialogBox.size(); // Update based on actual loaded texts, assuming each text is a "group" for now
    std::cout << "Dialog system initialized with " << _totalDialogGroups << " dialog text models." << std::endl;
}

void Dialog::proceed(const float &deltaTime) {
    // 如果暂停了，不处理任何逻辑
    if (_paused) {
        return;
    }

    // 仅更新淡出文本的生命周期
    updateTextLifetimes(deltaTime);
}

bool Dialog::isFinished() const {
    if (_started && _totalDialogGroups == 0) {
        return true;
    }
    return dialogBox.empty() && dropText.empty();
}

// bool Dialog::nextDialog() { ... } // Removed
// bool Dialog::shouldFinishCurrentDialog() const { ... } // Removed

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

void Dialog::draw(const float &deltaTime, GLSLProgram* shader, const glm::vec3& cameraPos, const glm::quat& cameraRot) {
    proceed(deltaTime);
    (void)cameraPos; // position not required when using camera rotation


    // Rotate texts to face the camera for better readability
    auto billboard = [&](std::unique_ptr<Text>& t){
        glm::vec3 dir = glm::normalize(cameraPos - t->getTransform().position);
        glm::quat look = glm::quatLookAt(dir, glm::vec3(0.0f, 1.0f, 0.0f)); // World UP is Y

        // This is the base rotation applied when the model was loaded (from Step 1)
        glm::quat base_orientation_at_load = glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // Rotation for upside-down effect (around model's local X-axis after base_orientation_at_load)
        // Assuming after base_orientation_at_load, the model's original X-axis is still its perceived horizontal axis.
        glm::quat upside_down_rotation = glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));

        // Combine base orientation with the upside-down rotation
        glm::quat final_base_orientation = upside_down_rotation * base_orientation_at_load;
        
        // The 'look' quaternion rotates the local -Z axis to 'dir'.
        // After 'base_orientation_at_load', let's assume the character's front is now along local +Z
        // (this depends on the original model orientation, if original front was +Y, after Rx(-90) it's +Z).
        // If the front is local +Z, then 'look * base_orientation_at_load' would make the back of the character face the camera.
        // To correct this, we can rotate by 180 degrees around the local Y axis of the character *after* the 'look' rotation,
        // or equivalently, before the 'look' rotation if we adjust what 'front' means.

        // Option: Rotate 180 degrees around world Y *after* 'look' has been applied to the base loaded orientation.
        // This should flip the text around if it's facing away, without causing mirroring if 'look' is correct.
        glm::quat flip_if_needed = glm::angleAxis(glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        
        t->getTransform().rotation = flip_if_needed * look * final_base_orientation;
    };

    for (auto& t : dialogBox) {
        billboard(t);
    }
    for (auto& t : dropText) {
        billboard(t);
    }

    drawDialogBox(shader);
    drawDropText(shader);
}

// size_t Dialog::getCurrentDialogIndex() const { ... } // Removed (member _currentDialogIndex removed from class)
// float Dialog::getCurrentDialogProgress() const { ... } // Removed (members _currentDialogIndex, continueTime removed from class)

size_t Dialog::getTotalDialogCount() const {
    return _totalDialogGroups;
}

void Dialog::forceNextDialog() {
    if (!dialogBox.empty()) {
        dropCurrentDialog(); // Move current texts to fade out
    }
    
    if (_config.loopDialogs) {
        resetDialog();
    } else {
        std::cout << "Dialog forced next; current content dropped. Not looping." << std::endl;
    }
}

void Dialog::resetDialog() {
    // 重置所有状态
    // _currentDialogIndex = 0; // Removed
    // _currentDialogTime = 0.0f; // Removed
    _started = true;
    _paused = false;
    
    // If there's an existing dialog in dialogBox, move it to dropText to fade out.
    // dropCurrentDialog() handles clearing dialogBox after moving elements.
    if (!dialogBox.empty()) {
        dropCurrentDialog();
    }
    // Note: We are not clearing dropText here, allowing previous text to fade out.
    // dialogBox is now empty.

    // preLoad.clear(); // Removed
    // continueTime.clear(); // Removed
    // _dialogFiles should already be populated by the constructor
    // If _dialogFiles is empty (e.g. constructor failed to load anything), 
    // this loop won't run, and dialogBox will remain empty.
    
    // New text layout logic for reset
    float currentLineWidth = 0.0f;
    float currentLineYOffset = 0.0f;

    for (const auto& filename : _dialogFiles) { // Iterate over previously loaded file names
        if (std::filesystem::exists(filename)) { // Should exist if loaded before, but good to check
            try {
                auto model = std::make_unique<AdvancedModel>(filename);
                auto text = std::make_unique<Text>(std::move(model));

                // Apply scale
                text->getTransform().scale = _config.textScale;

                // Apply base rotation (from Step 1)
                glm::quat baseRot =
                    glm::angleAxis(glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
                text->getTransform().rotation = baseRot;

                // Check for line wrapping
                if (currentLineWidth + _config.charSpacing > _config.lineMaxWidth && currentLineWidth > 0.0f) {
                    currentLineYOffset += _config.lineSpacing;
                    currentLineWidth = 0.0f;
                }

                // Set position
                text->getTransform().position =
                    _basePosition + glm::vec3(currentLineWidth, -currentLineYOffset, 0.0f);
                
                currentLineWidth += _config.charSpacing;

                dialogBox.push_back(std::move(text));
                // No need to add to _dialogFiles again, it's already populated

            } catch (const std::exception& e) {
                std::cerr << "Failed to reload dialog file " << filename << ": " << e.what() << std::endl;
            }
        }
    }
    _totalDialogGroups = dialogBox.size(); // Update based on reloaded texts
    std::cout << "Dialog system reset with " << _totalDialogGroups << " dialog text models." << std::endl;
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

    // Recalculate positions for texts in dialogBox based on the new layout logic
    float currentLineWidth = 0.0f;
    float currentLineYOffset = 0.0f;

    for (auto& text : dialogBox) {
        // Ensure scale is consistent if it were to change, though it's part of _config
        // text->getTransform().scale = _config.textScale; // Usually set at load time

        // Check for line wrapping
        // _config.charSpacing is used as the advance width for each character model
        if (currentLineWidth + _config.charSpacing > _config.lineMaxWidth && currentLineWidth > 0.0f) {
            currentLineYOffset += _config.lineSpacing;
            currentLineWidth = 0.0f;
        }

        // Set new position relative to the new _basePosition
        text->getTransform().position =
            _basePosition + glm::vec3(currentLineWidth, -currentLineYOffset, 0.0f);
        
        currentLineWidth += _config.charSpacing;
    }

    // TODO: dropText re-layout might be needed if its usage is maintained.
    // For now, its re-layout is omitted as its role might change in Step 6 (code simplification).
    // If dropText items need individual repositioning based on their original intended layout,
    // that would be more complex than a simple re-flow like dialogBox.
    // However, if dropText items are just fading out at their last known position,
    // their absolute positions might not need to change relative to the world,
    // or if they do, it's not a simple reflow.
    // Given the goal of "all words appear at once", the concept of dropText might be simplified/removed.
    // For now, let's focus on dialogBox.
    // If dropText elements are just individual words fading out, their positions are already world-space.
    // If _basePosition changes, they should probably also shift by the same delta as _basePosition.
    // This is simpler:
    // glm::vec3 delta = pos - oldBasePosition; // Need to store oldBasePosition or recalculate based on first char if possible
    // For now, let's assume dropText items are independent and their position doesn't get re-flowed here.
    // The original code for dropText in setBasePosition was likely flawed anyway by sharing 'offset'.
}

glm::vec3 Dialog::getBasePosition() const {
    return _basePosition;
}