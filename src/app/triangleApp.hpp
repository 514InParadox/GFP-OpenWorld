#ifndef _APP_TRIANGLE_H
#define _APP_TRIANGLE_H

#include <memory>

#include "model/triangle.hpp"
#include "utils/glsl_program.hpp"
#include "application.hpp"

class TriangleApp : public Application {
public:
    TriangleApp(const Options &options);

    ~TriangleApp() = default;

    void handleInput() override;
    
    void renderFrame() override;

private:
    std::unique_ptr<Model> _triangle;

    glm::vec3 _color;

    int twicks = 0;

    std::unique_ptr<GLSLProgram> _shader;

    void initShader();
};

#endif