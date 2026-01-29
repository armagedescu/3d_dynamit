#version 330 core
out vec4 FragColor;

in vec4 vertColor;
in vec3 vertNormal;
in vec3 fragPos;

uniform vec3 lightDir;
uniform vec3 viewPos;
uniform bool wireframeMode;

void main()
{
    if (wireframeMode)
    {
        FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    // Normalize inputs
    vec3 normal = normalize(vertNormal);
    vec3 lightDirection = normalize(-lightDir);

    // Ambient
    float ambientStrength = 0.3;
    vec3 ambient = ambientStrength * vertColor.rgb;

    // Diffuse
    float diff = max(dot(normal, lightDirection), 0.0);
    vec3 diffuse = diff * vertColor.rgb;

    // Specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - fragPos);
    vec3 reflectDir = reflect(-lightDirection, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * vec3(1.0);

    // Combine
    vec3 result = ambient + diffuse + specular;
    FragColor = vec4(result, vertColor.a);
}
