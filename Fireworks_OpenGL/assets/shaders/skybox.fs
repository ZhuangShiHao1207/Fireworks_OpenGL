#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    vec4 skyColor = texture(skybox, TexCoords);
    
    // 降低天空盒亮度，避免过亮
    // 可以根据需要调整这个值（0.0-1.0）
    float brightnessMultiplier = 0.6;  // 降低到 60% 亮度
    
    FragColor = vec4(skyColor.rgb * brightnessMultiplier, skyColor.a);
}
