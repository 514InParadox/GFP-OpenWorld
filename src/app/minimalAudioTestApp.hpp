#ifndef _APP_MINIMALAUDIOTEST_H
#define _APP_MINIMALAUDIOTEST_H

#include "application.hpp"

// irrKlang audio library
#include <irrklang/irrKlang.h>
using namespace irrklang;

class MinimalAudioTestApp : public Application {
public:    
    MinimalAudioTestApp(const Options &options);
    ~MinimalAudioTestApp();

    void handleInput() override;
    void renderFrame() override;

private:
    // irrKlang audio system
    ISoundEngine* _soundEngine = nullptr;
    
    // Simple state tracking
    bool _spacePressed = false;
    
    // Audio methods
    void initAudio();
    void cleanupAudio();
    void playTestSound();
};

#endif 