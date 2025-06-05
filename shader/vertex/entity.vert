#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;

out vec3 worldPosition;
out vec3 normal;
out vec2 texCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Transform vertex position to world space
    worldPosition = vec3(model * vec4(aPosition, 1.0));
    
    // Transform normal to world space (using normal matrix to handle non-uniform scaling)
    normal = mat3(transpose(inverse(model))) * aNormal;
    
    // Pass through texture coordinates
    texCoord = aTexCoord;
    
    // Transform to clip space
    gl_Position = projection * view * vec4(worldPosition, 1.0);
}