#version 330 core

out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D scene;      
uniform sampler2D bloomBlur;  
uniform bool useBloom;
uniform float exposure;

void main()
{
    vec3 hdr = texture(scene, TexCoords).rgb;
    vec3 bloom = texture(bloomBlur, TexCoords).rgb;

    vec3 color = hdr;
    if (useBloom) color += bloom * 2.0; 

    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    mapped = pow(mapped, vec3(1.0 / 2.2));

    FragColor = vec4(mapped, 1.0);
}