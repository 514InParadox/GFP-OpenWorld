#ifndef _UTILS_AUDIO_PLAYER_H
#define _UTILS_AUDIO_PLAYER_H

#include <string>
#include <unordered_map>
#include <memory>

// irrKlang audio library
#include <irrklang/irrKlang.h>
using namespace irrklang;

/**
 * AudioPlayer - A wrapper class for irrKlang audio engine
 * Provides easy-to-use interface for loading and playing audio files
 */
class AudioPlayer {
public:
    /**
     * Constructor - initializes the audio engine
     */
    AudioPlayer();
    
    /**
     * Destructor - cleans up audio resources
     */
    ~AudioPlayer();
    
    /**
     * Initialize the audio engine
     * @return true if successful, false otherwise
     */
    bool initialize();
    
    /**
     * Load an audio file into memory
     * @param name - identifier for the audio file
     * @param filePath - path to the audio file
     * @return true if loaded successfully, false otherwise
     */
    bool loadAudio(const std::string& name, const std::string& filePath);
    
    /**
     * Play an audio file
     * @param name - identifier of the loaded audio file
     * @param loop - whether to loop the audio (default: false)
     * @param volume - volume level (0.0 to 1.0, default: 1.0)
     * @return true if started playing successfully, false otherwise
     */
    bool playAudio(const std::string& name, bool loop = false, float volume = 1.0f);
    
    /**
     * Stop a currently playing audio
     * @param name - identifier of the audio to stop
     */
    void stopAudio(const std::string& name);
    
    /**
     * Stop all currently playing audio
     */
    void stopAllAudio();
    
    /**
     * Check if an audio is currently playing
     * @param name - identifier of the audio to check
     * @return true if playing, false otherwise
     */
    bool isPlaying(const std::string& name) const;
    
    /**
     * Set volume for a currently playing audio
     * @param name - identifier of the audio
     * @param volume - volume level (0.0 to 1.0)
     */
    void setVolume(const std::string& name, float volume);
    
    /**
     * Set master volume for all audio
     * @param volume - volume level (0.0 to 1.0)
     */
    void setMasterVolume(float volume);
    
    /**
     * Check if the audio engine is initialized
     * @return true if initialized, false otherwise
     */
    bool isInitialized() const { return _soundEngine != nullptr; }
    
    /**
     * Get the number of loaded audio files
     * @return number of loaded audio files
     */
    size_t getLoadedAudioCount() const { return _loadedAudio.size(); }

private:
    // irrKlang sound engine
    ISoundEngine* _soundEngine;
    
    // Structure to hold audio information
    struct AudioInfo {
        std::string filePath;
        ISound* currentSound;  // Currently playing instance (if any)
        
        AudioInfo(const std::string& path) : filePath(path), currentSound(nullptr) {}
    };
    
    // Map of loaded audio files
    std::unordered_map<std::string, std::unique_ptr<AudioInfo>> _loadedAudio;
    
    // Master volume
    float _masterVolume;
    
    /**
     * Clean up a specific audio info
     * @param audioInfo - pointer to the audio info to clean up
     */
    void cleanupAudioInfo(AudioInfo* audioInfo);
    
    /**
     * Validate volume value and clamp it to valid range
     * @param volume - volume to validate
     * @return clamped volume value
     */
    float validateVolume(float volume) const;
};

#endif 