#ifndef _APP_INITSCENEAPP_H
#define _APP_INITSCENEAPP_H

#include <memory>

#include "application.hpp"
#include "model/model.hpp"

class InitSceneApp : public Application {
public:    
    InitSceneApp(const Options &options);

    ~InitSceneApp() = default;

    void handleInput() override;

    void renderFrame() override;

private:
    std::unique_ptr<Model> _model;
};

#endif