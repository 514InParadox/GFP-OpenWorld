#include "dialog.hpp"
#include <iostream>
#include <chrono>
#include <thread>

// Example usage of the Dialog system
void dialogExample() {
    // Configure dialog behavior
    DialogConfig config;
    config.baseDisplayTime = 3.0f;     // 3 seconds base time
    config.timePerText = 1.5f;         // 1.5 seconds per text fragment
    config.fadeOutTime = 2.5f;         // 2.5 seconds fade out
    config.autoAdvance = true;         // Auto advance dialogs
    config.loopDialogs = false;        // Don't loop
    
    // Create dialog system with text files in "resource/dialog/" directory
    // Expects files like: resource/dialog/0-0.obj, resource/dialog/0-1.obj, resource/dialog/1-0.obj, etc.
    //Dialog dialogSystem("resource/dialog/", config);
    Dialog dialogSystem("resource/text/", config);
    
    std::cout << "Dialog system created with " << dialogSystem.getTotalDialogCount() << " dialog groups" << std::endl;
    
    // Simulate game loop
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (!dialogSystem.isFinished()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Update dialog system
        dialogSystem.proceed(deltaTime);
        
        // Example of drawing (in a real game, this would be in your render loop)
        std::cout << "Current dialog: " << dialogSystem.getCurrentDialogIndex() 
                  << "/" << dialogSystem.getTotalDialogCount() 
                  << " Progress: " << (dialogSystem.getCurrentDialogProgress() * 100) << "%" << std::endl;
        
        // Draw current dialog texts
        dialogSystem.drawDialogBox();
        
        // Draw fading out texts
        dialogSystem.drawDropText();
        
        // Simulate frame rate (60 FPS)
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
    
    std::cout << "Dialog finished!" << std::endl;
}

// Example with manual control
void manualDialogExample() {
    DialogConfig config;
    config.autoAdvance = false;  // Manual control
    
    //Dialog dialogSystem("resource/dialog/", config);
    Dialog dialogSystem("resource/text/", config);
    
    std::cout << "Manual dialog example - press Enter to advance dialogs" << std::endl;
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    
    while (!dialogSystem.isFinished()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Update dialog system (won't auto-advance)
        dialogSystem.proceed(deltaTime);
        
        // Draw dialogs
        dialogSystem.drawDialogBox();
        dialogSystem.drawDropText();
        
        // Check for user input (simplified - in real game use proper input system)
        if (std::cin.get()) {
            dialogSystem.forceNextDialog();
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}

// Example showing pause/resume functionality
void pauseResumeExample() {
    //Dialog dialogSystem("resource/dialog/");
    Dialog dialogSystem("resource/text/");
    
    auto lastTime = std::chrono::high_resolution_clock::now();
    bool paused = false;
    int frameCount = 0;

    
    while (!dialogSystem.isFinished()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;
        
        // Pause/resume every 3 seconds for demonstration
        frameCount++;
        if (frameCount % 180 == 0) { // Every 3 seconds at 60 FPS
            if (paused) {
                dialogSystem.resumeDialog();
                paused = false;
            } else {
                dialogSystem.pauseDialog();
                paused = true;
            }
        }
        
        dialogSystem.proceed(deltaTime);
        dialogSystem.drawDialogBox();
        dialogSystem.drawDropText();
        
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
