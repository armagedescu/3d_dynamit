#version 330 core
out vec4 FragColor;
in vec4 terrainColor;
in vec3 terrainNormal;
in vec3 lightDirection;
void main()
{
    float strength =  dot(normalize(-lightDirection), normalize(terrainNormal));
    FragColor = vec4( terrainColor.rgb * strength, terrainColor.a);
}
