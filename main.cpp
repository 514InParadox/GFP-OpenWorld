#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <memory>

#include "test/hello.hpp"

#include "utils/glsl_program.hpp"
#include "app/application.hpp"
#include "app/triangleApp.hpp"
#include "app/initSceneApp.hpp"
#include "app/cameraTestApp.hpp"
#include "app/testPhysicsApp.hpp"
#include "app/testPhysicsTwoApp.hpp"
#include "app/testPhysicsThreeApp.hpp"
#include "utils/collision_system.hpp"
#include "app/lightingTestApp.hpp"
#include "app/lightingTestApp2.hpp"

Options getOptions(int argc, char* argv[]) {
    Options options;
    options.windowTitle = "Physics Test App";
    options.windowWidth = 1280;
    options.windowHeight = 720;
    options.windowResizable = false;
    options.vSync = true;
    options.msaa = true;
    options.glVersion = {3, 3};
    // options.backgroundColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    options.backgroundColor = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
    // options.backgroundColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    options.assetRootDir = "./";

    return options;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

std::unique_ptr<GLSLProgram> shader;

int main(int argc, char *argv[]) {

    Options options = getOptions(argc, argv);

    try {
        // TriangleApp app(options);
         //InitSceneApp app(options);
         //CameraTestApp app(options);
        // TestPhysicsApp app(options);
        // TestPhysicsTwoApp app(options);
        //TestPhysicsThreeApp app(options);
        //lightingTestApp app(options);
        lightingTestApp2 app(options);
        app.run();
    } catch (std::runtime_error& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        exit(EXIT_FAILURE);
    } catch (...) {
        std::cerr << "Unknown Error" << std::endl;
        exit(EXIT_FAILURE);
    }

    return 0;
}