#version 330 core

in vec2 fTexCoord;

out vec4 color;

uniform sampler2D mapKds[2];

void main() {
    vec3 blendColor = texture(mapKds[0], fTexCoord).rgb * 0.5 + texture(mapKds[1], fTexCoord).rgb * 0.5;
    color = vec4(blendColor, 1.0f);
}