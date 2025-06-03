#version 330 core

// fragment shader for single dirlight/punctuallight/spotlight

// --- I/O ports ---
in vec3 worldPosition;
in vec3 normal;

out vec4 fragColor;

// -- structures --
struct Material {
    vec3 ambient;   // 环境光反射系数 Ka
    vec3 diffuse;   // 漫反射系数 Kd
    vec3 specular;  // 镜面反射系数 Ks
    vec3 color;     // 材质颜色
    float shininess; // 光泽度 Ns
};

struct AmbientLight{
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    // float kc, kl, kq;  // 用于计算光源衰减的常量。现在衰减我还搞不了
};

struct DirectionalLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float intensity;
};

struct SpotLight{
    vec3 position, direction;
    vec3 color;
    float cutOff, innerCutOff;
    float intensity;
};

// --- uniforms ---
uniform Material material;
uniform AmbientLight ambientLight;
uniform PointLight pointLights;
uniform DirectionalLight dirLights;
uniform SpotLight spotLights;

// uniform vec3 objectColor;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 viewPosition; // 摄像机在世界空间中的位置


// -- Functions --

// Punctual light
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 worldPosition, vec3 viewDir) {

    vec3 lightDir = normalize(light.position - worldPosition);

    // Diffuse
    
    vec3 diffuse = light.color * max(dot(normalize(normal), lightDir), 0.0f) * material.diffuse;
    
    // Specular
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normalize(normal), halfVector), 0.0), material.shininess);
    vec3 specular = light.color * spec * material.specular;
    
    // Attenuation, for now ignore it
    float attenuation = 1.0f;

    // float distance = length(light.position - worldPosition);
    // float attenuation = 1.0 / (light.kc + light.kl * distance + light.kq * (distance * distance));
    return light.intensity * attenuation * (diffuse + specular);
}

// Directional light
vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 worldPosition, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);

    vec3 allnull = vec3(0.0f ,0.0f ,0.0f); 
    vec3 diffuse_null = vec3(1.0f ,0.0f ,0.0f);
    vec3 specular_null = vec3(0.0f ,0.0f ,1.0f);

    // Diffuse
    vec3 diffuse = light.color * max(dot(normalize(normal), lightDir), 0.0f) * material.diffuse;
    
    // Specular
    vec3 halfVector = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normalize(normal), halfVector), 0.0), material.shininess);
    vec3 specular = light.color * spec * material.specular;

    return light.intensity * (diffuse + specular);
}

// Spotlight

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 worldPosition, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - worldPosition);
    float theta = dot(lightDir, normalize(-light.direction)); // 光线方向与聚光灯方向的点积

    // Check if object is in the range of spotlight
    if (theta > light.cutOff) {
        // Diffuse
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = light.color * diff * material.diffuse;
        // Speccular

        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), material.shininess);
        vec3 specular = light.color * spec * material.specular;

        // attenuation, let's just ignore it okay
        // float distance = length(light.position - worldPosition);
        // float attenuation = 1.0 / (light.kc + light.kl * distance + light.kq * (distance * distance));
        float attenuation = 1.0;

        // soft edge of light
        // float epsilon = light.cutOff - light.innerCutOff;
        // float intensityFactor = clamp((theta - light.innerCutOff) / epsilon, 0.0, 1.0);
        float intensityFactor = 1.0; // No soft edge

        return light.intensity * attenuation * intensityFactor * (diffuse + specular);
    } else {
        // Out of range
        return vec3(0.0, 0.0, 1.0);
    }
}

void main() {
    // parameters for debuggin
    // vec3 lightPosition = vec3(0.0f, 0.0f, -10.0f);
    // vec3 objectColor = vec3(0.8667, 0.6353, 0.0941);

    // parameters
    vec3 viewDirection = normalize(viewPosition - worldPosition);
    vec3 result;

    // 1. Ambient light
    vec3 ambient = material.ambient * material.color * ambientLight.color * ambientLight.intensity;
    result += ambient;

    // 2. Point light
    vec3 pointLight = CalcPointLight(pointLights, normal, worldPosition, viewDirection);
    result += pointLight;

    // 3. Directional light

    vec3 dirLight = CalcDirectionalLight(dirLights, normal, worldPosition, viewDirection);
    result += dirLight;

    // 4. Spotlight
    vec3 spotlight = CalcSpotLight(spotLights, normal, worldPosition, viewDirection);
    result += spotlight;

    fragColor = vec4(result, 1.0f);

}

