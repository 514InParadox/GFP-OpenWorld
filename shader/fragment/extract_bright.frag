#version 330 core
layout(location = 0) out vec4 brightColorMap;

uniform sampler2D sceneMap;

in vec2 screenTexCoord;

void main() {
    vec3 sceneColor = texture(sceneMap, screenTexCoord).rgb;
    float luminance = dot(sceneColor, vec3(0.299, 0.587, 0.114));
    
    vec3 brightColor = vec3(0.0);
    if (luminance > 1.0) brightColor = sceneColor;

    brightColorMap = vec4(brightColor, 1.0);
} 