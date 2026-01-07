#version 430 core
layout (location = 0) in vec3 vert;
layout (location = 1) in vec3 norm;
layout (location = 2) in vec4 vertColor;

uniform mat4 model;      //takes local coordinates for thing and moves it into world coordinates
uniform mat4 view;       //moves world space objects around based on camera
uniform mat4 projection; //converts values to normalised device coordinates (use sweet math for perspective)

out vec4 tcsTerrainColor;
out vec3 tcsTerrainNormal;
out vec3 tcsLightDirection;

void main()
{
	gl_Position       = projection * view * model * vec4(vert, 1.0);
	tcsTerrainColor   = vertColor;
	tcsTerrainNormal  = norm;
	tcsLightDirection = vec3(0.f, -1.f, 0.f);
}
