#version 330 core

// Fragment shader for entity models with complete lighting support

// --- Input from vertex shader ---
in vec3 worldPosition;
in vec3 normal;
in vec2 texCoord;
in mat3 TBN;

// --- Output ---
out vec4 fragColor;

// --- Material structure ---
struct Material {
    vec3 ambient;    // Ka - ambient reflection coefficient
    vec3 diffuse;    // Kd - diffuse reflection coefficient  
    vec3 specular;   // Ks - specular reflection coefficient
    vec3 color;      // material base color
    float shininess; // Ns - shininess/gloss factor
};

// --- Light structures ---
struct AmbientLight {
    vec3 color;
    float intensity;
};

struct PointLight {
    vec3 position;
    vec3 color;
    float intensity;
    // Attenuation factors could be added here: float kc, kl, kq;
};

struct DirectionalLight {
    vec3 position;   // Not used for directional lights
    vec3 direction;
    vec3 color;
    float intensity;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 color;
    float cutOff;        // Outer cone angle (cosine)
    float innerCutOff;   // Inner cone angle (cosine) - optional
    float intensity;
};

// --- Uniforms ---
#define MAX_POINT_LIGHTS 32

uniform Material material;
uniform AmbientLight ambientLight;
uniform PointLight pointLights[MAX_POINT_LIGHTS];
uniform int numPointLights;
uniform DirectionalLight dirLights;
uniform SpotLight spotLights;

uniform vec3 viewPosition;  // Camera position in world space

// Optional texture support
uniform sampler2D diffuseTexture;
uniform sampler2D normalTexture;
uniform bool useTexture = false;
uniform bool useNormalTexture = false;

// --- Lighting calculation functions ---

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 worldPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - worldPos);
    
    // Diffuse shading
    float diff = max(dot(normalize(normal), lightDir), 0.0);
    vec3 diffuse = light.color * diff * material.diffuse;
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normalize(normal), halfwayDir), 0.0), material.shininess);
    vec3 specular = light.color * spec * material.specular;
    
    // Enhanced distance-based attenuation for better light falloff
    float distance = length(light.position - worldPos);
    float attenuation = 1.0 / (1.0 + 0.7 * distance + 1.8 * distance * distance + distance * distance * distance);
    
    return light.intensity * attenuation * (diffuse + specular);
}

vec3 CalcDirectionalLight(DirectionalLight light, vec3 normal, vec3 worldPos, vec3 viewDir) {
    vec3 lightDir = normalize(-light.direction);
    
    // Diffuse shading
    float diff = max(dot(normalize(normal), lightDir), 0.0);
    vec3 diffuse = light.color * diff * material.diffuse;
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normalize(normal), halfwayDir), 0.0), material.shininess);
    vec3 specular = light.color * spec * material.specular;
    
    return light.intensity * (diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 worldPos, vec3 viewDir) {
    vec3 lightDir = normalize(light.position - worldPos);
    float theta = dot(lightDir, normalize(-light.direction));
    
    // Check if fragment is within spotlight cone
    if (theta > light.cutOff) {
        // Diffuse shading
        float diff = max(dot(normalize(normal), lightDir), 0.0);
        vec3 diffuse = light.color * diff * material.diffuse;
        
        // Specular shading (Blinn-Phong)
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normalize(normal), halfwayDir), 0.0), material.shininess);
        vec3 specular = light.color * spec * material.specular;
        
        // Attenuation
        float attenuation = 1.0;
        
        // Soft edges (optional - using simplified version)
        float intensityFactor = 1.0;
        // float epsilon = light.cutOff - light.innerCutOff;
        // float intensityFactor = clamp((theta - light.innerCutOff) / epsilon, 0.0, 1.0);
        
        return light.intensity * attenuation * intensityFactor * (diffuse + specular);
    } else {
        // Outside spotlight cone
        return vec3(0.0);
    }
}

void main() {
    // Get normal from normal map or use vertex normal
    vec3 norm;
    if (useNormalTexture) {
        // Sample normal from normal map and transform to [-1, 1] range
        vec3 normalMap = texture(normalTexture, texCoord).rgb * 2.0 - 1.0;
        // Transform normal from tangent space to world space using TBN matrix
        norm = normalize(TBN * normalMap);
    } else {
        // Use vertex normal
        norm = normalize(normal);
    }
    
    vec3 viewDir = normalize(viewPosition - worldPosition);
    
    // Get base color (from texture or material color)
    vec3 baseColor = material.color;
    if (useTexture) {
        baseColor *= texture(diffuseTexture, texCoord).rgb;
    }
    
    // Start with ambient lighting
    vec3 result = material.ambient * baseColor * ambientLight.color * ambientLight.intensity;
    
    // Add all point lights contribution
    for (int i = 0; i < numPointLights && i < MAX_POINT_LIGHTS; i++) {
        result += CalcPointLight(pointLights[i], norm, worldPosition, viewDir);
    }
    
    // Add directional light contribution  
    result += CalcDirectionalLight(dirLights, norm, worldPosition, viewDir);
    
    // Add spotlight contribution
    result += CalcSpotLight(spotLights, norm, worldPosition, viewDir);
    
    // Apply base color to the lighting result
    result *= baseColor;
    
    fragColor = vec4(result, 1.0);
    // fragColor = vec4(1.0, 1.0, 1.0, 1.0);
}