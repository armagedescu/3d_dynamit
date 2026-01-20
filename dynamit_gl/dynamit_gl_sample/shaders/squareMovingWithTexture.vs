#version 330 core

//values
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec2 aTexCoord; //texture coordinate for this vertex

//info we want to pass onto next shader
out vec4 vertexColour;
out vec2 TexCoord;

uniform mat4 transform; //matrix for translation, scale and rotation

void main(){
	gl_Position = transform* vec4(aPos, 1.0);
	
	vertexColour = vec4(aColour, 1);
	TexCoord = aTexCoord;
}