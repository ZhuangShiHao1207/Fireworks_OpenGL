#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;

// Ground texture
uniform sampler2D groundTexture;
uniform bool useTexture;

// Simple ground material
uniform vec3 groundColor;

// Material properties for specular reflection
uniform float groundShininess = 32.0;     // Ground is less shiny than model
uniform float groundSpecularStrength = 0.3; // Ground reflects less light

// Fog parameters for infinite ground illusion
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogStart;

// Point lights (for fireworks - maximum 16 lights)
#define MAX_LIGHTS 16
uniform int numLights;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];
uniform float lightIntensities[MAX_LIGHTS];

vec3 CalcPointLight(vec3 lightPos, vec3 lightColor, float intensity, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor)
{
    vec3 lightDir = normalize(lightPos - fragPos);
    
    // Diffuse shading (Lambertian)
    float diff = max(dot(normal, lightDir), 0.0);
    
    // Specular shading (Blinn-Phong)
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float spec = pow(max(dot(normal, halfwayDir), 0.0), groundShininess);
    
    // Attenuation
    float distance = length(lightPos - fragPos);
    float attenuation = intensity / (1.0 + 0.09 * distance + 0.032 * (distance * distance));
    
    // Combine results
    vec3 ambient = 0.05 * lightColor;
    vec3 diffuse = diff * lightColor;
    vec3 specular = spec * lightColor * groundSpecularStrength;
    
    // Apply attenuation
    ambient *= attenuation * baseColor;
    diffuse *= attenuation * baseColor;
    specular *= attenuation;  // Specular is additive
    
    return ambient + diffuse + specular;
}

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    // Get base color (from texture or uniform)
    vec3 baseColor = useTexture ? texture(groundTexture, TexCoords).rgb : groundColor;
    
    // Base ambient color for ground
    vec3 result = baseColor * 0.15; // Low ambient
    
    // Calculate lighting from all active point lights
    for(int i = 0; i < numLights && i < MAX_LIGHTS; i++)
    {
        result += CalcPointLight(lightPositions[i], lightColors[i], lightIntensities[i], norm, FragPos, viewDir, baseColor);
    }
    
    // Simple grid pattern (optional visual enhancement) - only if not using texture
    if (!useTexture) {
        float gridScale = 2.0;
        vec2 gridCoord = fract(TexCoords * gridScale);
        float grid = step(0.95, gridCoord.x) + step(0.95, gridCoord.y);
        result = mix(result, result * 1.2, grid * 0.3);
    }
    
    // ? 改进的雾效计算 - 更平滑的地平线过渡
    float distance = length(viewPos - FragPos);
    
    // 计算视角高度因子（越接近地平线，雾效越强）
    vec3 viewToFrag = FragPos - viewPos;
    float heightFactor = 1.0 - abs(normalize(viewToFrag).y);  // 接近水平视角时接近1
    heightFactor = pow(heightFactor, 2.0);  // 增强地平线效果
    
    // 混合距离雾和高度雾
    float distanceFog = 0.0;
    if (distance > fogStart) {
        float fogDistance = distance - fogStart;
        distanceFog = 1.0 - exp(-fogDensity * fogDistance);
    }
    
    // 添加额外的地平线雾效
    float horizonFog = smoothstep(20.0, 60.0, distance) * heightFactor * 0.7;
    
    // 组合雾效
    float fogFactor = clamp(distanceFog + horizonFog, 0.0, 1.0);
    
    // 使用更柔和的雾色混合
    vec3 finalFogColor = fogColor;
    
    // 在远处添加一点天空盒颜色的过渡（假设天空是深蓝色）
    if (distance > 40.0) {
        float skyMix = smoothstep(40.0, 80.0, distance);
        // 天空盒底部的颜色（根据你的星空天空盒调整）
        vec3 skyHorizonColor = vec3(0.02, 0.05, 0.15);  // 深蓝偏黑
        finalFogColor = mix(fogColor, skyHorizonColor, skyMix * 0.5);
    }
    
    // Mix result with fog color based on distance
    result = mix(result, finalFogColor, fogFactor);
    
    FragColor = vec4(result, 1.0);
}
