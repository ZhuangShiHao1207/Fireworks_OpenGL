#version 330 core
out vec4 FragColor;

// 输入：bright pass 结果
in vec2 TexCoords;

uniform sampler2D image;
// 配合ping pong使用
uniform bool horizontal;

const float weight[5] = float[](0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main()
{
    // 单个像素偏移
    vec2 tex_offset = 1.0 / textureSize(image, 0); 
    // 中心像素权重
    vec3 result = texture(image, TexCoords).rgb * weight[0];

    // 对水平 or 垂直方向进行一次高斯模糊
    for(int i = 1; i < 5; ++i)
    {
        if(horizontal)
        {
            result += texture(image, TexCoords + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
        else
        {
            result += texture(image, TexCoords + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(image, TexCoords - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    // 得到模糊后的亮部图
    FragColor = vec4(result, 1.0);
}
