#pragma once

#include "transform.hpp"

struct Light {
    Transform transform;
    float intensity = 1.0f;
    glm::vec3 color = {1.0f, 1.0f, 1.0f};
    virtual ~Light(){}
};

struct AmbientLight : public Light {};

struct DirectionalLight : public Light {};
// The transform->direction represents the direcion of the lights

struct PointLight : public Light {
    float kc = 1.0f;
    float kl = 0.0f;
    float kq = 1.0f;
};

struct SpotLight : public Light {
    float cutOff = glm::radians(60.0f);
    float innerCutOff = glm::radians(40.0f);
    float kc = 1.0f;
    float kl = 0.0f;
    float kq = 1.0f;
};

struct Material {
    glm::vec3 ambient;   // 环境光反射系数 Ka
    glm::vec3 diffuse;   // 漫反射系数 Kd
    glm::vec3 specular;  // 镜面反射系数 Ks
    float shininess;  // 光泽度 Ns
    glm::vec3 materialColor;
    Material(){
        ambient = glm::vec3(0.1f,0.1f,0.1f);
        diffuse = glm::vec3(0.8f,0.8f,0.8f);
        specular = glm::vec3(0.5f,0.5f,0.5f);
        shininess = 32.0f;
        materialColor = glm::vec3(0.8667, 0.6353, 0.0941);
    }
}; // The material for light test.