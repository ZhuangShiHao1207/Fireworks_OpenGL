#version 330 core
in vec4 particleColor;

out vec4 FragColor;

void main()
{
    // 创建圆形粒子
    vec2 coord = gl_PointCoord - vec2(0.5);
    float dist = length(coord);
    
    if (dist > 0.5) {
        discard; // 丢弃圆外的片段
    }
    
    // 添加一些边缘柔和效果
    float alpha = 1.0 - smoothstep(0.3, 0.5, dist);
    
    FragColor = vec4(particleColor.rgb, particleColor.a * alpha);
}