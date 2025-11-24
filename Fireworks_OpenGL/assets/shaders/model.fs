#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

// Material textures
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_specular1;
uniform bool hasTexture;

// Lighting
uniform vec3 viewPos;

// Material properties for specular reflection
uniform float materialShininess = 64.0;  // Shininess of the material (higher = shinier)
uniform float specularStrength = 0.5;    // Strength of specular highlights (0.0 to 1.0)

// Point lights (for fireworks and scene lighting - maximum 16 lights)
#define MAX_LIGHTS 16
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];
uniform float lightIntensities[MAX_LIGHTS];

// Fog parameters
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogStart;

// Calculate point light contribution using Blinn-Phong
vec3 CalcPointLight(vec3 lightPos, vec3 lightColor, float intensity, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // Diffuse shading (Lambertian)
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading (Blinn-Phong with enhanced visibility)
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(normal, halfwayDir), 0.0), materialShininess);
    
    // Attenuation (inverse square law with linear term)
    float distance = length(lightPos - fragPos);
    float attenuation = intensity / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    
    // Combine results - Enhanced specular for better visibility
    vec3 ambient = 0.02 * lightColor;
    vec3 diffuse = diff * lightColor * 0.8;
    vec3 specular = spec * lightColor * specularStrength;  // Use configurable specular strength
    
    // Apply attenuation and base color
    ambient *= attenuation * baseColor;
    diffuse *= attenuation * baseColor;
    specular *= attenuation;  // Specular is additive, not multiplied by base color
    
    return ambient + diffuse + specular;
}

void main()
{    
    // Base color from texture or default
    vec3 color;
    if (hasTexture) {
        vec4 texColor = texture(texture_diffuse1, TexCoords);
        color = texColor.rgb;
        
        // If texture is invalid (all zeros or very dark), use fallback
        if (length(color) < 0.01) {
            color = vec3(0.5, 0.6, 0.5); // Fallback color
        }
    } else {
        color = vec3(0.5, 0.6, 0.5); // Default greenish color for island
    }
    
    // Ensure normal is normalized (important for correct lighting)
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Base ambient lighting
    vec3 result = color * 0.1;
    
    // Calculate lighting from all active point lights
    for(int i = 0; i < numLights && i < MAX_LIGHTS; i++)
    {
        result += CalcPointLight(lightPositions[i], lightColors[i], lightIntensities[i], norm, FragPos, viewDir, color);
    }
    
    // Clamp to prevent over-saturation
    result = clamp(result, 0.0, 1.0);
    
    // Apply fog
    float distance = length(viewPos - FragPos);
    float fogFactor = 0.0;
    
    if (distance > fogStart) {
        float fogDistance = distance - fogStart;
        fogFactor = 1.0 - exp(-fogDensity * fogDistance);
        fogFactor = clamp(fogFactor, 0.0, 1.0);
    }
    
    result = mix(result, fogColor, fogFactor);
    
    FragColor = vec4(result, 1.0);
}
