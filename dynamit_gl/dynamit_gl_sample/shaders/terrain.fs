#version 330 core
out vec4 color;

in vec4 terrainColor;
in vec3 terrainNormal;
in vec3 lightDirection;

void main()
{
    float strength  =  dot(-lightDirection, terrainNormal);
	color = vec4(terrainColor.rgb * strength, terrainColor.a);
}
