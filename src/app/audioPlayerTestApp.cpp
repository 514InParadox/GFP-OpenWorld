#include "audioPlayerTestApp.hpp"
#include <iostream>

AudioPlayerTestApp::AudioPlayerTestApp(const Options &options) : Application(options) {
    // Initialize audio player
    if (!_audioPlayer.initialize()) {
        std::cerr << "Failed to initialize AudioPlayer!" << std::endl;
        return;
    }
    
    // Load audio files
    initializeAudio();
    
    // Display help
    displayHelp();
}

void AudioPlayerTestApp::initializeAudio() {
    std::cout << "\n=== Loading Audio Files ===" << std::endl;
    
    // Load different audio files
    _audioPlayer.loadAudio("walking", "audio/walking-sound.wav");
    _audioPlayer.loadAudio("patrol", "audio/patrol.mp3");
    _audioPlayer.loadAudio("chase", "audio/chase.mp3");
    
    std::cout << "Loaded " << _audioPlayer.getLoadedAudioCount() << " audio files" << std::endl;
}

void AudioPlayerTestApp::displayHelp() {
    std::cout << "\n=== AudioPlayer Test App ===" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  1 - Play walking sound (loop)" << std::endl;
    std::cout << "  2 - Play patrol music (loop)" << std::endl;
    std::cout << "  3 - Play chase music (once)" << std::endl;
    std::cout << "  SPACE - Stop all audio" << std::endl;
    std::cout << "  S - Stop current walking sound" << std::endl;
    std::cout << "  + - Increase volume" << std::endl;
    std::cout << "  - - Decrease volume" << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "Current volume: " << _currentVolume << std::endl;
    std::cout << "==============================" << std::endl;
}

void AudioPlayerTestApp::handleInput() {
    // Exit if ESC is pressed
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    // Play walking sound (loop)
    if (_input.keyboard.keyStates[GLFW_KEY_1] == GLFW_PRESS && !_key1Pressed) {
        _key1Pressed = true;
        if (_audioPlayer.isPlaying("walking")) {
            std::cout << "Walking sound is already playing" << std::endl;
        } else {
            _audioPlayer.playAudio("walking", true, _currentVolume); // loop = true
        }
    } else if (_input.keyboard.keyStates[GLFW_KEY_1] == GLFW_RELEASE) {
        _key1Pressed = false;
    }
    
    // Play patrol music (loop)
    if (_input.keyboard.keyStates[GLFW_KEY_2] == GLFW_PRESS && !_key2Pressed) {
        _key2Pressed = true;
        if (_audioPlayer.isPlaying("patrol")) {
            std::cout << "Patrol music is already playing" << std::endl;
        } else {
            _audioPlayer.playAudio("patrol", true, _currentVolume); // loop = true
        }
    } else if (_input.keyboard.keyStates[GLFW_KEY_2] == GLFW_RELEASE) {
        _key2Pressed = false;
    }
    
    // Play chase music (once)
    if (_input.keyboard.keyStates[GLFW_KEY_3] == GLFW_PRESS && !_key3Pressed) {
        _key3Pressed = true;
        _audioPlayer.playAudio("chase", false, _currentVolume); // loop = false
    } else if (_input.keyboard.keyStates[GLFW_KEY_3] == GLFW_RELEASE) {
        _key3Pressed = false;
    }
    
    // Stop all audio
    if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS && !_spacePressed) {
        _spacePressed = true;
        _audioPlayer.stopAllAudio();
    } else if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_RELEASE) {
        _spacePressed = false;
    }
    
    // Stop walking sound specifically
    if (_input.keyboard.keyStates[GLFW_KEY_S] == GLFW_PRESS && !_stopPressed) {
        _stopPressed = true;
        _audioPlayer.stopAudio("walking");
    } else if (_input.keyboard.keyStates[GLFW_KEY_S] == GLFW_RELEASE) {
        _stopPressed = false;
    }
    
    // Volume up
    if (_input.keyboard.keyStates[GLFW_KEY_EQUAL] == GLFW_PRESS && !_volumeUpPressed) { // + key
        _volumeUpPressed = true;
        _currentVolume = std::min(1.0f, _currentVolume + 0.1f);
        _audioPlayer.setMasterVolume(_currentVolume);
        std::cout << "Volume: " << _currentVolume << std::endl;
    } else if (_input.keyboard.keyStates[GLFW_KEY_EQUAL] == GLFW_RELEASE) {
        _volumeUpPressed = false;
    }
    
    // Volume down
    if (_input.keyboard.keyStates[GLFW_KEY_MINUS] == GLFW_PRESS && !_volumeDownPressed) { // - key
        _volumeDownPressed = true;
        _currentVolume = std::max(0.0f, _currentVolume - 0.1f);
        _audioPlayer.setMasterVolume(_currentVolume);
        std::cout << "Volume: " << _currentVolume << std::endl;
    } else if (_input.keyboard.keyStates[GLFW_KEY_MINUS] == GLFW_RELEASE) {
        _volumeDownPressed = false;
    }
    
    _input.forwardState();
}

void AudioPlayerTestApp::renderFrame() {
    // Simple dark blue background
    glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
} 