#version 330 core

in vec3 worldPosition;
in vec3 normal;
in vec2 fTexCoord;

out vec4 fragColor;

uniform vec3 objectColor;
uniform sampler2D mapKd;

void main() {
    vec3 lightPosition = vec3(100.0f, 100.0f, 100.0f);

    // ambient color
    float ka = 0.1f;
    vec3 ambient = ka * objectColor;

    // diffuse color
    float kd = 0.8f;
    vec3 lightColor = vec3(1.0f, 1.0f, 1.0f);
    vec3 lightDirection = normalize(lightPosition - worldPosition);
    vec3 diffuse = kd * lightColor * max(dot(normalize(normal), lightDirection), 0.0f);
    
    vec3 texColor = texture(mapKd, fTexCoord).rgb; 

    // fragColor = vec4(texColor * (ambient + diffuse), 1.0f);
    // fragColor = vec4(ambient + diffuse, 1.0f);
    fragColor = vec4(texColor, 1.0f);
}