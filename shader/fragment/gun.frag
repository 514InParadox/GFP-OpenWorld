#version 330 core

// Fragment shader for gun model with diffuse and normal texture support

// --- Input from vertex shader ---
in vec3 worldPosition;
in vec3 normal;
in vec2 texCoord;
in vec3 tangent;
in vec3 bitangent;
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

// Texture support for gun
uniform sampler2D diffuseTexture;    // Color/diffuse texture
uniform sampler2D normalTexture;     // Normal map texture
uniform bool useDiffuseTexture = false;
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
    float attenuation = 1.0 / (1.0 + 0.35 * distance + 0.44 * distance * distance);
    
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
        
        return light.intensity * attenuation * intensityFactor * (diffuse + specular);
    } else {
        // Outside spotlight cone
        return vec3(0.0);
    }
}

void main() {
    // Get normal from normal map if available, otherwise use interpolated vertex normal
    vec3 norm;
    if (useNormalTexture) {
        // Sample normal from normal map
        vec3 normalMapSample = texture(normalTexture, texCoord).rgb;
        // Convert from [0,1] to [-1,1] range
        normalMapSample = normalMapSample * 2.0 - 1.0;
        // Transform from tangent space to world space using TBN matrix
        norm = normalize(TBN * normalMapSample);
    } else {
        // Use interpolated vertex normal
        norm = normalize(normal);
    }
    
    vec3 viewDir = normalize(viewPosition - worldPosition);
    
    // Get base color (from diffuse texture or material color)
    vec3 baseColor = material.color;
    if (useDiffuseTexture) {
        vec4 textureColor = texture(diffuseTexture, texCoord);
        baseColor *= textureColor.rgb;
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
}
