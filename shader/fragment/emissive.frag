#version 330 core

in vec3 worldPosition;
in vec3 normal;

out vec4 fragColor;

struct Material {
    vec3 color;
    float intensity;
};

uniform Material material;

void main() {
    // Simple emissive material - just output the color with intensity
    fragColor = vec4(material.color * material.intensity, 1.0);
} 