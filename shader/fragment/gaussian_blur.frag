#version 330 core
out vec4 FragColor;

in vec2 screenTexCoord;

uniform sampler2D image;
uniform bool horizontal;

const float weight[7] = float[] (
    0.1964825501f, 0.1946178343f, 0.1680312748f, 0.1208413362f, 0.0732200231f, 0.0365513677f, 0.0152822971f);

void main() {
    vec2 tex_offset = 1.0 / textureSize(image, 0);
    vec3 result = texture(image, screenTexCoord).rgb * weight[0];
    
    if (horizontal) {
        for (int i = 1; i < 7; ++i) {
            float offset = float(i) * tex_offset.x;
            
            result += texture(image, screenTexCoord + vec2(offset, 0.0)).rgb * weight[i];
            result += texture(image, screenTexCoord - vec2(offset, 0.0)).rgb * weight[i];
        }
    } else {
        for (int i = 1; i < 7; ++i) {
            float offset = float(i) * tex_offset.y;
            
            result += texture(image, screenTexCoord + vec2(0.0, offset)).rgb * weight[i];
            result += texture(image, screenTexCoord - vec2(0.0, offset)).rgb * weight[i];
        }
    }
    
    FragColor = vec4(result, 1.0);
} 