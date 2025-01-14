#version 450 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTexCoord;

out vec3 color;
out vec2 texCoord;

uniform float scale;

void main()
{
    gl_Position = vec4(aPos.x * scale, aPos.y * scale, aPos.z, 1.0);
    color = aColor;
//    color = vec3(aTexCoord, 0.f);
    texCoord = aTexCoord;
}