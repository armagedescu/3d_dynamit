#version 330 core

//value passed in via vertex array object using a vertex buffer object
layout (location = 0) in vec3 aPos;

//this variable will be passed out to the fragment shader
out vec4 vertexColour;

void main()
{
	//Vertex shader MUST set gl_Position for shape assembly, its a vec4
	gl_Position = vec4(aPos, 1.0);
	
	//setting a value for vertexColour
	vertexColour = vec4(1.0, 1.0, 0.0, 1.0);
}