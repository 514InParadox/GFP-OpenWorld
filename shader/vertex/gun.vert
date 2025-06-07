#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;   // Tangent vector for normal mapping

out vec3 worldPosition;
out vec3 normal;
out vec2 texCoord;
out vec3 tangent;
out vec3 bitangent;
out mat3 TBN;  // Tangent-Bitangent-Normal matrix for transforming normals

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // Transform vertex position to world space
    worldPosition = vec3(model * vec4(aPosition, 1.0));
    
    // Transform normal to world space (using normal matrix to handle non-uniform scaling)
    vec3 worldNormal = normalize(mat3(transpose(inverse(model))) * aNormal);
    normal = worldNormal;
    
    // Transform tangent to world space
    vec3 worldTangent = normalize(mat3(model) * aTangent);
    tangent = worldTangent;
    
    // Calculate bitangent in world space
    // Re-orthogonalize tangent with respect to normal
    worldTangent = normalize(worldTangent - dot(worldTangent, worldNormal) * worldNormal);
    vec3 worldBitangent = cross(worldNormal, worldTangent);
    bitangent = worldBitangent;
    
    // Create TBN matrix for transforming normals from tangent space to world space
    TBN = mat3(worldTangent, worldBitangent, worldNormal);
    
    // Pass through texture coordinates
    texCoord = aTexCoord;
    
    // Transform to clip space
    gl_Position = projection * view * vec4(worldPosition, 1.0);
}
