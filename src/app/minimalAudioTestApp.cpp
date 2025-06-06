#include "minimalAudioTestApp.hpp"
#include <iostream>

MinimalAudioTestApp::MinimalAudioTestApp(const Options &options) : Application(options) {
    // Initialize audio system
    initAudio();
    
    std::cout << "=== Minimal Audio Test App ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  SPACE - Play audio" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "Audio file: audio/walking-sound.mp3" << std::endl;
    std::cout << "===============================" << std::endl;
}

MinimalAudioTestApp::~MinimalAudioTestApp() {
    cleanupAudio();
}

void MinimalAudioTestApp::initAudio() {
    // Initialize irrKlang sound engine
    _soundEngine = createIrrKlangDevice();
    
    if (!_soundEngine) {
        std::cerr << "Error: Could not initialize irrKlang audio engine!" << std::endl;
        return;
    }
    
    std::cout << "irrKlang audio engine initialized successfully!" << std::endl;
}

void MinimalAudioTestApp::cleanupAudio() {
    if (_soundEngine) {
        _soundEngine->drop(); // Release the sound engine
        _soundEngine = nullptr;
        std::cout << "Audio engine cleaned up." << std::endl;
    }
}

void MinimalAudioTestApp::handleInput() {
    // Exit if ESC is pressed
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    // Play audio when SPACE is pressed (only once per press)
    if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS && !_spacePressed) {
        _spacePressed = true;
        playTestSound();
    } else if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_RELEASE) {
        _spacePressed = false;
    }
    
    _input.forwardState();
}

void MinimalAudioTestApp::playTestSound() {
    if (!_soundEngine) {
        std::cout << "No audio engine available!" << std::endl;
        return;
    }
    
    const char* audioFile = "audio/walking-sound.wav";
    
    std::cout << "Playing audio: " << audioFile << std::endl;
    
    // Play the sound (2D, not looped, return sound object)
    ISound* sound = _soundEngine->play2D(audioFile, false, false, true);
    
    if (sound) {
        // Set volume to a reasonable level
        sound->setVolume(0.7f);
        
        // Release reference (sound will continue playing)
        sound->drop();
        
        std::cout << "✓ Audio started playing" << std::endl;
    } else {
        std::cout << "✗ Failed to play audio file: " << audioFile << std::endl;
        std::cout << "  Make sure the file exists and is a valid audio format" << std::endl;
    }
}

void MinimalAudioTestApp::renderFrame() {
    // Simple black screen
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
} 