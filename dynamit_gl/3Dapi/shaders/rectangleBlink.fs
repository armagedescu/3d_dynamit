#version 330

out vec4 zizi;

//global variable that stays the same between calls to this shader
//which we can set via code
uniform vec4 ziziIn;

void main()
{
	zizi = ziziIn;
}