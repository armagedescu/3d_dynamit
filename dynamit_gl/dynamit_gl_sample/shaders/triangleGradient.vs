#version 330 core

//values passed in
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColour;

//pass out variables to next shader
out vec4 vertexColour;

void main()
{
	gl_Position = vec4(aPos, 1.0);
	vertexColour = vec4(aColour, 1.0);
}