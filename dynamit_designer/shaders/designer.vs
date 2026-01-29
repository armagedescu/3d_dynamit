#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec4 aColor;

out vec4 vertColor;
out vec3 vertNormal;
out vec3 fragPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    fragPos = vec3(model * vec4(aPos, 1.0));
    vertNormal = mat3(transpose(inverse(model))) * aNormal;
    vertColor = aColor;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
