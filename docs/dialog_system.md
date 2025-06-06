# Dialog System Documentation

## Overview

The Dialog system manages text-based conversations in the game. It loads 3D text models from files, displays them in sequence with configurable timing, and handles the lifecycle from display to fade-out.

## File Structure

Dialog text files should be organized with the naming convention:
```
{assetPath}{groupIndex}-{textIndex}.obj
```

For example:
```
resource/dialog/0-0.obj  // First text of first dialog group
resource/dialog/0-1.obj  // Second text of first dialog group
resource/dialog/1-0.obj  // First text of second dialog group
resource/dialog/1-1.obj  // Second text of second dialog group
resource/dialog/1-2.obj  // Third text of second dialog group
```

## Basic Usage

### 1. Create a Dialog System

```cpp
#include "dialog.hpp"

// Simple creation with default settings
Dialog dialogSystem("resource/dialog/");

// Or with custom configuration
DialogConfig config;
config.baseDisplayTime = 3.0f;     // Base display time per dialog group
config.timePerText = 1.5f;         // Additional time per text fragment
config.fadeOutTime = 2.5f;         // Time for fade-out effect
config.autoAdvance = true;         // Automatically advance dialogs
config.loopDialogs = false;        // Don't repeat dialogs

Dialog dialogSystem("resource/dialog/", config);
```

### 2. Update in Game Loop

```cpp
void gameLoop() {
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (!dialogSystem.isFinished()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Update dialog system
        dialogSystem.proceed(deltaTime);
        
        // Render dialogs
        dialogSystem.drawDialogBox();    // Draw current dialog
        dialogSystem.drawDropText();     // Draw fading out text
        
        // Other game logic...
    }
}
```

### 3. Manual Control

```cpp
DialogConfig config;
config.autoAdvance = false;  // Disable auto-advance

Dialog dialogSystem("resource/dialog/", config);

// In your input handling:
if (playerPressedAdvanceKey) {
    dialogSystem.forceNextDialog();
}

// Pause/resume functionality:
if (playerPressedPauseKey) {
    dialogSystem.pauseDialog();
}
if (playerPressedResumeKey) {
    dialogSystem.resumeDialog();
}
```

## Configuration Options

### DialogConfig Structure

```cpp
struct DialogConfig {
    float baseDisplayTime = 2.0f;    // Base time for each dialog group
    float timePerText = 1.0f;        // Additional time per text fragment
    float fadeOutTime = 2.0f;        // Extra time for fade out effect
    bool autoAdvance = true;         // Whether to auto-advance dialogs
    bool loopDialogs = false;        // Whether to loop through dialogs
};
```

### Configuration Examples

```cpp
// Fast-paced dialog
DialogConfig fastConfig;
fastConfig.baseDisplayTime = 1.0f;
fastConfig.timePerText = 0.5f;
fastConfig.fadeOutTime = 1.0f;

// Slow, cinematic dialog
DialogConfig cinematicConfig;
cinematicConfig.baseDisplayTime = 5.0f;
cinematicConfig.timePerText = 2.0f;
cinematicConfig.fadeOutTime = 3.0f;

// Manual control for interactive dialog
DialogConfig interactiveConfig;
interactiveConfig.autoAdvance = false;
```

## API Reference

### Core Methods

- `Dialog(const std::string& assetPath, const DialogConfig& config = {})` - Constructor
- `void proceed(const float& deltaTime)` - Update dialog system
- `bool isFinished() const` - Check if all dialogs are complete
- `void drawDialogBox() const` - Draw current dialog texts
- `void drawDropText() const` - Draw fading out texts

### Control Methods

- `void forceNextDialog()` - Manually advance to next dialog
- `void resetDialog()` - Reset to beginning
- `void pauseDialog()` - Pause dialog progression
- `void resumeDialog()` - Resume dialog progression

### Information Methods

- `size_t getCurrentDialogIndex() const` - Get current dialog index
- `size_t getTotalDialogCount() const` - Get total number of dialog groups
- `float getCurrentDialogProgress() const` - Get progress (0.0-1.0) of current dialog

### Configuration Methods

- `void setConfig(const DialogConfig& config)` - Update configuration
- `const DialogConfig& getConfig() const` - Get current configuration

## Dialog Lifecycle

1. **PreLoad**: Text models are loaded from files into memory
2. **DialogBox**: Current dialog texts are displayed and updated
3. **DropText**: Finished dialog texts fade out over time
4. **Cleanup**: Expired texts are removed from memory

## Integration with Text Class

The Dialog system works with the `Text` class which wraps `AdvancedModel`:

```cpp
class Text {
public:
    Text(std::unique_ptr<AdvancedModel> text);
    void setLifeTime(const float& lifeTime);
    bool Life(const float& deltaTime);  // Returns false when expired
    void draw();                        // Render the text model
};
```

## Performance Considerations

- Text models are loaded once during initialization
- Only active dialog texts are rendered each frame
- Expired texts are automatically cleaned up
- Memory usage scales with the number of dialog groups and texts per group

## Error Handling

- Missing dialog files are logged but don't crash the system
- Invalid model files are caught and logged
- The system gracefully handles empty dialog directories
- Dialog progression continues even if some texts fail to load

## Example Project Structure

```
resource/
  dialog/
    intro/
      0-0.obj  // "Welcome to"
      0-1.obj  // "the game!"
      1-0.obj  // "Press any key"
      1-1.obj  // "to continue"
    level1/
      0-0.obj  // First dialog of level 1
      0-1.obj  // Second part of first dialog
      1-0.obj  // Second dialog
```

## Best Practices

1. **Organize by Context**: Group related dialogs in subdirectories
2. **Consistent Naming**: Follow the {group}-{text}.obj convention
3. **Performance**: Limit text complexity for real-time rendering
4. **Testing**: Test with different dialog lengths and configurations
5. **Localization**: Consider directory structure for multiple languages
