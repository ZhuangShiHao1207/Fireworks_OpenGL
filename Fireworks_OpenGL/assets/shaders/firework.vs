#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec4 aColor;
layout (location = 2) in float aSize;

out vec4 particleColor;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
    particleColor = aColor;
    // 使用传入的粒子大小；乘以缩放因子以匹配像素级尺寸
    gl_PointSize = max(1.0, aSize * 200.0);
}