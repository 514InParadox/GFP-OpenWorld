#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoord;
layout(location = 3) in vec3 aTangent;
layout(location = 4) in vec3 aBitangent;
layout(location = 5) in ivec4 aBoneIds; 
layout(location = 6) in vec4 aWeights;

out vec3 worldPosition;
out vec3 normal;
out vec2 texCoord;
out mat3 TBN;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// NOTE: This must match MAX_BONES in animation_data.hpp
const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

void main() {
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
    vec3 totalTangent = vec3(0.0f);
    vec3 totalBitangent = vec3(0.0f);
    float totalWeight = 0.0f;
    bool hasBoneInfluence = false;
    
    // Calculate bone-influenced position and normal
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(aBoneIds[i] == -1 || aWeights[i] <= 0.0f) 
            continue;
        if(aBoneIds[i] < 0 || aBoneIds[i] >= MAX_BONES) 
            continue;
            
        hasBoneInfluence = true;
        
        vec4 localPosition = finalBonesMatrices[aBoneIds[i]] * vec4(aPosition, 1.0f);
        totalPosition += localPosition * aWeights[i];
        
        vec3 localNormal = mat3(finalBonesMatrices[aBoneIds[i]]) * aNormal;
        totalNormal += localNormal * aWeights[i];
        
        vec3 localTangent = mat3(finalBonesMatrices[aBoneIds[i]]) * aTangent;
        totalTangent += localTangent * aWeights[i];
        
        vec3 localBitangent = mat3(finalBonesMatrices[aBoneIds[i]]) * aBitangent;
        totalBitangent += localBitangent * aWeights[i];
        
        totalWeight += aWeights[i];
    }
    
    // If no bone influence or weights don't sum to 1, use original position
    if (!hasBoneInfluence || totalWeight < 0.01f) {
        totalPosition = vec4(aPosition, 1.0f);
        totalNormal = aNormal;
        totalTangent = aTangent;
        totalBitangent = aBitangent;
    } else {
        // Normalize by total weight to handle cases where weights don't sum to exactly 1
        totalPosition /= totalWeight;
        totalNormal /= totalWeight;
        totalTangent /= totalWeight;
        totalBitangent /= totalWeight;
    }
    
    // Transform vertex position to world space
    worldPosition = vec3(model * totalPosition);
    
    // Transform normal to world space (using normal matrix to handle non-uniform scaling)
    mat3 normalMatrix = mat3(transpose(inverse(model)));
    normal = normalMatrix * normalize(totalNormal);
    
    // Calculate TBN matrix for normal mapping
    vec3 T = normalize(normalMatrix * totalTangent);
    vec3 B = normalize(normalMatrix * totalBitangent);
    vec3 N = normalize(normal);
    TBN = mat3(T, B, N);
    
    // Pass through texture coordinates
    texCoord = aTexCoord;
    
    // Transform to clip space
    gl_Position = projection * view * vec4(worldPosition, 1.0);
}
