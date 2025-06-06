#ifndef _APP_AUDIOPLAYERTEST_H
#define _APP_AUDIOPLAYERTEST_H

#include "application.hpp"
#include "utils/audio_player.hpp"

class AudioPlayerTestApp : public Application {
public:    
    AudioPlayerTestApp(const Options &options);
    ~AudioPlayerTestApp() = default;

    void handleInput() override;
    void renderFrame() override;

private:
    // Audio player instance
    AudioPlayer _audioPlayer;
    
    // Key state tracking to prevent repeated actions
    bool _key1Pressed = false;
    bool _key2Pressed = false;
    bool _key3Pressed = false;
    bool _spacePressed = false;
    bool _stopPressed = false;
    bool _volumeUpPressed = false;
    bool _volumeDownPressed = false;
    
    // Volume control
    float _currentVolume = 0.7f;
    
    // Initialize audio files
    void initializeAudio();
    
    // Display help information
    void displayHelp();
};

#endif 