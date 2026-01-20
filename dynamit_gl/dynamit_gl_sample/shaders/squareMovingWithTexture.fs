#version 330 core
out vec4 FragColour;

//get values from vertex shader
in vec4 vertexColour;
in vec2 TexCoord;

//sampler holds reference to texture slot we want to use
uniform sampler2D texture1airplane;
uniform sampler2D texture0crate;

void main(){
	//will get relevant pixels from texture based on 'fragments' trying to 
	//render and TexCoord
	FragColour = mix(texture(texture1airplane, TexCoord), texture(texture0crate, TexCoord), 0.5) * vertexColour;
	//FragColour = texture(texture1airplane, TexCoord) * vertexColour;
}