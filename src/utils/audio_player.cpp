#include "audio_player.hpp"
#include <iostream>
#include <algorithm>

AudioPlayer::AudioPlayer() : _soundEngine(nullptr), _masterVolume(1.0f) {
    // Constructor - initialization is done in initialize() method
}

AudioPlayer::~AudioPlayer() {
    // Clean up all audio resources
    stopAllAudio();
    
    // Clean up loaded audio info
    _loadedAudio.clear();
    
    // Clean up sound engine
    if (_soundEngine) {
        _soundEngine->drop();
        _soundEngine = nullptr;
    }
}

bool AudioPlayer::initialize() {
    if (_soundEngine) {
        std::cout << "AudioPlayer: Already initialized" << std::endl;
        return true;
    }
    
    // Create irrKlang sound engine
    _soundEngine = createIrrKlangDevice();
    
    if (!_soundEngine) {
        std::cerr << "AudioPlayer: Failed to initialize irrKlang sound engine" << std::endl;
        return false;
    }
    
    std::cout << "AudioPlayer: Successfully initialized irrKlang sound engine" << std::endl;
    return true;
}

bool AudioPlayer::loadAudio(const std::string& name, const std::string& filePath) {
    if (!_soundEngine) {
        std::cerr << "AudioPlayer: Sound engine not initialized" << std::endl;
        return false;
    }
    
    if (name.empty()) {
        std::cerr << "AudioPlayer: Audio name cannot be empty" << std::endl;
        return false;
    }
    
    // Check if audio with this name already exists
    if (_loadedAudio.find(name) != _loadedAudio.end()) {
        std::cout << "AudioPlayer: Audio '" << name << "' already loaded, replacing..." << std::endl;
        // Stop and clean up existing audio
        stopAudio(name);
    }
    
    // Test if the file can be loaded by trying to play it (paused)
    ISound* testSound = _soundEngine->play2D(filePath.c_str(), false, true, true); // paused = true
    if (!testSound) {
        std::cerr << "AudioPlayer: Failed to load audio file: " << filePath << std::endl;
        return false;
    }
    
    // Stop the test sound immediately
    testSound->stop();
    testSound->drop();
    
    // Create audio info and store it
    _loadedAudio[name] = std::make_unique<AudioInfo>(filePath);
    
    std::cout << "AudioPlayer: Successfully loaded audio '" << name << "' from " << filePath << std::endl;
    return true;
}

bool AudioPlayer::playAudio(const std::string& name, bool loop, float volume) {
    if (!_soundEngine) {
        std::cerr << "AudioPlayer: Sound engine not initialized" << std::endl;
        return false;
    }
    
    auto it = _loadedAudio.find(name);
    if (it == _loadedAudio.end()) {
        std::cerr << "AudioPlayer: Audio '" << name << "' not found. Please load it first." << std::endl;
        return false;
    }
    
    AudioInfo* audioInfo = it->second.get();
    
    // Stop any currently playing instance of this audio
    if (audioInfo->currentSound && !audioInfo->currentSound->isFinished()) {
        audioInfo->currentSound->stop();
        audioInfo->currentSound->drop();
        audioInfo->currentSound = nullptr;
    }
    
    // Play the audio
    audioInfo->currentSound = _soundEngine->play2D(audioInfo->filePath.c_str(), loop, false, true);
    
    if (!audioInfo->currentSound) {
        std::cerr << "AudioPlayer: Failed to play audio '" << name << "'" << std::endl;
        return false;
    }
    
    // Set volume (combine with master volume)
    float finalVolume = validateVolume(volume) * _masterVolume;
    audioInfo->currentSound->setVolume(finalVolume);
    
    std::cout << "AudioPlayer: Playing '" << name << "' (loop: " << (loop ? "yes" : "no") 
              << ", volume: " << finalVolume << ")" << std::endl;
    
    return true;
}

void AudioPlayer::stopAudio(const std::string& name) {
    auto it = _loadedAudio.find(name);
    if (it == _loadedAudio.end()) {
        std::cout << "AudioPlayer: Audio '" << name << "' not found" << std::endl;
        return;
    }
    
    AudioInfo* audioInfo = it->second.get();
    cleanupAudioInfo(audioInfo);
    
    std::cout << "AudioPlayer: Stopped audio '" << name << "'" << std::endl;
}

void AudioPlayer::stopAllAudio() {
    for (auto& pair : _loadedAudio) {
        cleanupAudioInfo(pair.second.get());
    }
    
    std::cout << "AudioPlayer: Stopped all audio" << std::endl;
}

bool AudioPlayer::isPlaying(const std::string& name) const {
    auto it = _loadedAudio.find(name);
    if (it == _loadedAudio.end()) {
        return false;
    }
    
    AudioInfo* audioInfo = it->second.get();
    return audioInfo->currentSound && !audioInfo->currentSound->isFinished();
}

void AudioPlayer::setVolume(const std::string& name, float volume) {
    auto it = _loadedAudio.find(name);
    if (it == _loadedAudio.end()) {
        std::cout << "AudioPlayer: Audio '" << name << "' not found" << std::endl;
        return;
    }
    
    AudioInfo* audioInfo = it->second.get();
    if (audioInfo->currentSound && !audioInfo->currentSound->isFinished()) {
        float finalVolume = validateVolume(volume) * _masterVolume;
        audioInfo->currentSound->setVolume(finalVolume);
        
        std::cout << "AudioPlayer: Set volume for '" << name << "' to " << finalVolume << std::endl;
    }
}

void AudioPlayer::setMasterVolume(float volume) {
    _masterVolume = validateVolume(volume);
    
    // Update volume for all currently playing audio
    for (auto& pair : _loadedAudio) {
        AudioInfo* audioInfo = pair.second.get();
        if (audioInfo->currentSound && !audioInfo->currentSound->isFinished()) {
            // Note: We don't know the original volume, so we just apply master volume
            // In a more sophisticated implementation, we'd store the original volume
            audioInfo->currentSound->setVolume(_masterVolume);
        }
    }
    
    std::cout << "AudioPlayer: Set master volume to " << _masterVolume << std::endl;
}

void AudioPlayer::cleanupAudioInfo(AudioInfo* audioInfo) {
    if (audioInfo && audioInfo->currentSound) {
        if (!audioInfo->currentSound->isFinished()) {
            audioInfo->currentSound->stop();
        }
        audioInfo->currentSound->drop();
        audioInfo->currentSound = nullptr;
    }
}

float AudioPlayer::validateVolume(float volume) const {
    return std::clamp(volume, 0.0f, 1.0f);
} 