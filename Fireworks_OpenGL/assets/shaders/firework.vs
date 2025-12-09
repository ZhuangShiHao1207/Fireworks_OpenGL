#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aSize;

out vec4 particleColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec4 viewPos = view * vec4(aPos, 1.0);
    gl_Position = projection * viewPos;
    particleColor = aColor;
    
    // 粒子大小基于距离：靠近时变大（透视效果）
    // 使用viewPos.z的绝对值作为距离
    float distance = length(viewPos.xyz);
    float sizeScale = 200.0 / max(distance, 1.0); // 距离越近，比例越大
    gl_PointSize = max(1.0, aSize * sizeScale);
}