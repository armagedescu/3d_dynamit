#version 430 core
out vec4 color;
in vec4 tes_color;
void main()
{
	color = tes_color;
}