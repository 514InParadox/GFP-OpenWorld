#version 330 core

in vec2 TexCoord;

out vec4 FragColor;

// Texture sampler
uniform sampler2D textureImage;

void main() {
    vec4 texColor = texture(textureImage, TexCoord);
    FragColor = texColor;
}