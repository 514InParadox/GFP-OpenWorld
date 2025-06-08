#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIds; 
layout (location = 6) in vec4 aWeights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

// NOTE: This must match MAX_BONES in animation_data.hpp
const int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform mat4 finalBonesMatrices[MAX_BONES];

out vec2 TexCoords;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    vec4 totalPosition = vec4(0.0f);
    vec3 totalNormal = vec3(0.0f);
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
        
        vec4 localPosition = finalBonesMatrices[aBoneIds[i]] * vec4(aPos, 1.0f);
        totalPosition += localPosition * aWeights[i];
        
        vec3 localNormal = mat3(finalBonesMatrices[aBoneIds[i]]) * aNormal;
        totalNormal += localNormal * aWeights[i];
        
        totalWeight += aWeights[i];
    }
    
    // If no bone influence or weights don't sum to 1, use original position
    if (!hasBoneInfluence || totalWeight < 0.01f) {
        totalPosition = vec4(aPos, 1.0f);
        totalNormal = aNormal;
    } else {
        // Normalize by total weight to handle cases where weights don't sum to exactly 1
        totalPosition /= totalWeight;
        totalNormal /= totalWeight;
    }
		
    mat4 viewModel = view * model;
    gl_Position = projection * viewModel * totalPosition;
    
    FragPos = vec3(model * totalPosition);
    Normal = mat3(transpose(inverse(model))) * normalize(totalNormal);
    TexCoords = aTexCoords;
}
