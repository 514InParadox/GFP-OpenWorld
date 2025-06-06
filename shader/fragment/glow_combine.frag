#version 330 core
out vec4 FragColor;

in vec2 screenTexCoord;

uniform sampler2D sceneTexture;
uniform sampler2D bloomTexture;
uniform bool enableGlow;
uniform float glowIntensity;

void main() {
    vec3 sceneColor = texture(sceneTexture, screenTexCoord).rgb;
    vec3 bloomColor = texture(bloomTexture, screenTexCoord).rgb;
    
    if (enableGlow) {
        // Add bloom effect to the scene
        vec3 result = sceneColor + bloomColor * glowIntensity;
        FragColor = vec4(result, 1.0);
    } else {
        // Just show the original scene
        FragColor = vec4(sceneColor, 1.0);
    }
} 