#version 330 core

//values passed in
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColour;

uniform mat4 model;      //takes local coordinates for thing and moves it into world coordinates
uniform mat4 view;       //moves world space objects around based on camera
uniform mat4 projection; //converts values to normalised device coordinates (use sweet math for perspective)


//pass out variables to next shader
out vec4 vertexColour;

void main()
{
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	vertexColour = vec4(aColour, 1.0);
}