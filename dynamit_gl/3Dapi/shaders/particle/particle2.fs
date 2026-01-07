#version 330 core

in vec2 UV;
in vec4 particlecolor;

out vec4 color;

uniform sampler2D particleTexture;

void main()
{
	color = texture( particleTexture, UV ) * particlecolor;
}