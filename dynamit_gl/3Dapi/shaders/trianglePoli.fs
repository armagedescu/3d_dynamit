#version 330
//Fragment must pass OUT a vec4 to describe final pixel colour
out vec4 FragColour;

//catch values passed out from vertex shader with in variables
in vec4 vertexColour;

void main()
{
	FragColour = vertexColour.xyzw;
}