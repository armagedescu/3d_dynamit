#version 330 core

//values passed in
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec2 aTexCoord; //texture coordinate for this vertex

out vec4 vertexColour;
out vec2 TexCoord;

uniform mat4 model;      //takes local coordinates for thing and moves it into world coordinates
uniform mat4 view;       //moves world space objects around based on camera
uniform mat4 projection; //converts values to normalised device coordinates (use sweet math for perspective)

void main()
{
	//note: remember matrices multiply right to left
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	
	vertexColour = vec4(aColour,1.0);
	TexCoord = aTexCoord;
}