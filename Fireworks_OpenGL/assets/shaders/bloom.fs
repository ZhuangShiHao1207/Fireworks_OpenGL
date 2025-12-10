#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D scene;

// 亮度阈值，超过这个值的像素会被认为是亮部 --> 若不是HDR，则整个屏幕都会被过滤掉 --> 全黑
const float threshold = 1.5;

void main()
{
    vec3 color = texture(scene, TexCoords).rgb;
    // 人眼感知亮度公式
    float brightness = dot(color, vec3(0.2126, 0.7152, 0.0722)); // 感知亮度
    // 只提取出亮部，将其他部分设为黑色
    // 最终结果为：原场景 + 模糊处理后的亮部
    if(brightness > threshold)
        FragColor = vec4(color, 1.0);
    else
        // 置为黑色
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
}
