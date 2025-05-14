#include "lightingTestApp.hpp"


const std::string modelPath = "resource/model/MonkeyHead.obj";

const std::string vertexShaderAddr   = "shader/vertex/initSceneApp.vert";
const std::string fragmentShaderAddr = "shader/fragment/lighting.frag";

const std::vector<std::string> skyboxTexturePaths = {
    "resource/texture/skybox/default/right.jpg",
    "resource/texture/skybox/default/left.jpg",
    "resource/texture/skybox/default/top.jpg",
    "resource/texture/skybox/default/bottom.jpg",
    "resource/texture/skybox/default/front.jpg",
    "resource/texture/skybox/default/back.jpg"
};
 
lightingTestApp::lightingTestApp(const Options &options) : Application(options) {
    // set input mode
    glfwSetInputMode(_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    _input.mouse.move.xNow = _input.mouse.move.xOld = 0.5f * _windowWidth;
    _input.mouse.move.yNow = _input.mouse.move.yOld = 0.5f * _windowHeight;
    glfwSetCursorPos(_window, _input.mouse.move.xNow, _input.mouse.move.yNow);

    // init cameras
    _cameras.resize(2);

    const float aspect = 1.0f * _windowWidth / _windowHeight;
    constexpr float znear = 0.1f;
    constexpr float zfar = 10000.0f;

    // perspective camera
    _cameras[0].reset(new PerspectiveCamera(glm::radians(60.0f), aspect, 0.1f, 10000.0f));
    _cameras[0]->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);

    // orthographic camera
    _cameras[1].reset(
        new OrthographicCamera(-4.0f * aspect, 4.0f * aspect, -4.0f, 4.0f, znear, zfar));
    _cameras[1]->transform.position = glm::vec3(0.0f, 0.0f, 15.0f);

    // init model
    _model.reset(new Model(getAssetFullPath(modelPath)));

    // init shader
    initShader();

    // init skybox
    std::vector<std::string> _skyboxTexturePaths;
    for (int i = 0; i < skyboxTexturePaths.size(); ++i) {
        _skyboxTexturePaths.push_back(getAssetFullPath(skyboxTexturePaths[i]));
    }
    _skybox.reset(new SkyBox(_skyboxTexturePaths));

    // init light
    _light = std::make_unique<Light>();
    _light->transform.position = glm::vec3(100.0f, 100.0f, 100.0f); // default light position


    // init material
    _testMaterial = std::make_unique<Material>();
}

void lightingTestApp::handleInput() { 
    constexpr float cameraMoveSpeed = 0.5f;
    constexpr float cameraRotateSpeed = 0.002f;
    constexpr float lightMoveSpeed = 2.0f;
    if (_input.keyboard.keyStates[GLFW_KEY_ESCAPE] != GLFW_RELEASE) {
        glfwSetWindowShouldClose(_window, true);
        return;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_SPACE] == GLFW_PRESS) {
        activeCameraIndex = (activeCameraIndex + 1) % _cameras.size();
        _input.keyboard.keyStates[GLFW_KEY_SPACE] = GLFW_RELEASE;
        return;
    }

    Camera* camera = _cameras[activeCameraIndex].get();

    if (_input.keyboard.keyStates[GLFW_KEY_W] != GLFW_RELEASE) {
        camera->transform.position = camera->transform.position + cameraMoveSpeed * camera->transform.getFront();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_A] != GLFW_RELEASE) {
        camera->transform.position = camera->transform.position - cameraMoveSpeed * camera->transform.getRight();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_S] != GLFW_RELEASE) {
        camera->transform.position = camera->transform.position - cameraMoveSpeed * camera->transform.getFront();
    }

    if (_input.keyboard.keyStates[GLFW_KEY_D] != GLFW_RELEASE) {
        camera->transform.position = camera->transform.position + cameraMoveSpeed * camera->transform.getRight();
    }

    if (_input.mouse.move.xNow != _input.mouse.move.xOld) {
        float mouse_movement_in_x_direction = _input.mouse.move.xNow - _input.mouse.move.xOld;
        float delta = cameraRotateSpeed * mouse_movement_in_x_direction;

        camera->transform.rotation = glm::angleAxis(-delta, glm::vec3(0.0f, 1.0f, 0.0f)) * camera->transform.rotation;
    }

    if (_input.mouse.move.yNow != _input.mouse.move.yOld) {
        float mouse_movement_in_y_direction = _input.mouse.move.yNow - _input.mouse.move.yOld;
        float delta = cameraRotateSpeed * mouse_movement_in_y_direction;

        camera->transform.rotation = glm::angleAxis(-delta, camera->transform.getRight()) * camera->transform.rotation;
    }

    // Use arrow key to move lights

    if (_input.keyboard.keyStates[GLFW_KEY_UP] != GLFW_RELEASE) {
        _light->transform.position = _light->transform.position + lightMoveSpeed * _light->transform.getDefaultUp();
        std::cerr << "Light move up to (" 
        << _light->transform.position.x << ", " 
        << _light->transform.position.y << ", "
        << _light->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_LEFT] != GLFW_RELEASE) {
        _light->transform.position = _light->transform.position - lightMoveSpeed * _light->transform.getDefaultRight();
        std::cerr << "Light move left to (" 
        << _light->transform.position.x << ", " 
        << _light->transform.position.y << ", "
        << _light->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_DOWN] != GLFW_RELEASE) {
        _light->transform.position = _light->transform.position - lightMoveSpeed * _light->transform.getDefaultUp();
        std::cerr << "Light move down to (" 
        << _light->transform.position.x << ", " 
        << _light->transform.position.y << ", "
        << _light->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_RIGHT] != GLFW_RELEASE) {
        _light->transform.position = _light->transform.position + lightMoveSpeed * _light->transform.getDefaultRight();
        std::cerr << "Light move right to (" 
        << _light->transform.position.x << ", " 
        << _light->transform.position.y << ", "
        << _light->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_COMMA] != GLFW_RELEASE) {
        _light->transform.position = _light->transform.position - lightMoveSpeed * _light->transform.getDefaultFront();
        std::cerr << "Light move nearar to (" 
        << _light->transform.position.x << ", " 
        << _light->transform.position.y << ", "
        << _light->transform.position.z << ")"
        << std::endl;
    }

    if (_input.keyboard.keyStates[GLFW_KEY_PERIOD] != GLFW_RELEASE) {
        _light->transform.position = _light->transform.position + lightMoveSpeed * _light->transform.getDefaultFront();
        std::cerr << "Light move farther to (" 
        << _light->transform.position.x << ", " 
        << _light->transform.position.y << ", "
        << _light->transform.position.z << ")"
        << std::endl;
    }

    _input.forwardState();
}

void lightingTestApp::renderFrame() {   
    glClearColor(_clearColor.r, _clearColor.g, _clearColor.b, _clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    glm::mat4 projection = _cameras[activeCameraIndex]->getProjectionMatrix();
    glm::mat4 view = _cameras[activeCameraIndex]->getViewMatrix();

    _shader->use();

    // Initialize testing material
    
    _shader->setUniformVec3("objectColor",_testMaterial->materialColor);
    _shader->setUniformVec3("ka",_testMaterial->ambient);
    _shader->setUniformVec3("kd",_testMaterial->diffuse);
    _shader->setUniformVec3("ks",_testMaterial->specular);
    _shader->setUniformFloat("shininess",_testMaterial->shininess);    
    
    // Initialize light
    
    
    _shader->setUniformVec3("lightColor",_light->color);
    _shader->setUniformVec3("lightPosition",_light->transform.position);

    _shader->setUniformVec3("viewPosition",_cameras[activeCameraIndex]->transform.position);

    _shader->setUniformMat4("projection", projection);
    _shader->setUniformMat4("view", view);
    _shader->setUniformMat4("model", _model->transform.getLocalMatrix());
    
    _model->draw();

    _skybox->draw(projection, view);
}

void lightingTestApp::initShader() {
    _shader.reset(new GLSLProgram);
    _shader->attachVertexShaderFromFile(getAssetFullPath(vertexShaderAddr));
    _shader->attachFragmentShaderFromFile(getAssetFullPath(fragmentShaderAddr));
    _shader->link();
}

// void lightingTestApp::setMaterial(){
//     glm::vec3 ambient = glm::vec3(0.5f,0.5f,0.5f);
//     glm::vec3 diffuse = glm::vec3(0.5f,0.5f,0.5f);
//     glm::vec3 specular = glm::vec3(0.5f,0.5f,0.5f);
//     float shininess = 0.8f;
//     _shader->setUniformVec3("material.ambient",ambient);
//     _shader->setUniformVec3("material.diffuse", diffuse);
//     _shader->setUniformVec3("material.specular", specular);
//     _shader->setUniformFloat("material.shininess", shininess);
// }

// void lightingTestApp::setLight(){
//     glm::vec3 lightColor = glm::vec3(1.0f,1.0f,1.0f);
//     // Set ambient
//     _shader->setUniformVec3("ambientLight.color",lightColor);
//     _shader->setUniformFloat("ambient.intensity",0.1f);
//     // Set directional light
//     _shader->setUniformVec3("dirLights[0].color",lightColor);
//     // _shader->setUniformVec3("dirLight[0].direction")
//     // Set punctual light
// }