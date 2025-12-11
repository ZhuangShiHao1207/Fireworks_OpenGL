#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

// 纹理采样器（直接从初始化，只会在外部设置赋值）
uniform sampler2D scene;      
uniform sampler2D bloomBlur;  
uniform bool useBloom;
uniform float exposure;
uniform float fadeAlpha; // 淡入淡出透明度（0.0 = 全黑，1.0 = 正常）

void main()
{
    vec3 hdr = texture(scene, TexCoords).rgb;
    vec3 bloom = texture(bloomBlur, TexCoords).rgb;

    vec3 color = hdr;
    if (useBloom) color += bloom * 2; 

    // 应用色调映射和 Gamma 校正
    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    mapped = pow(mapped, vec3(1.0 / 2.2));
    
    // 应用淡入淡出效果（混合到黑色）
    mapped = mix(vec3(0.0), mapped, fadeAlpha);

    FragColor = vec4(mapped, 1.0);
}