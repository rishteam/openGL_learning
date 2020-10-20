#version 450 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

uniform mat4 vModel;
uniform mat4 vView;
uniform mat4 vProjection;

uniform vec3 lightPos;

out vec3 LightPos;
out vec3 FragPos;
out vec3 Normal;

void main()
{
    gl_Position = vProjection * vView * vModel * vec4(aPos, 1.0);
    FragPos = vec3(vView * vModel * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(vView * vModel))) * aNormal;
    LightPos = vec3(vView * vec4(lightPos, 1.0));
}