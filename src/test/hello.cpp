#include "hello.hpp"
#include <stdio.h>

void printHello() {
    glm::vec3 a{1.0f, 1.0f, 1.0f};    
    glm::quat rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    glm::vec3 scale = {1.0f, 1.0f, 1.0f};
    printf("Hello %f\n", a[0]);
}