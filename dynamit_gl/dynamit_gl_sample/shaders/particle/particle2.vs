#version 330 core

// Input vertex data, different for all executions of this shader.
layout(location = 0) in vec3 squareVertices; //Vertices, relative to the center
layout(location = 1) in vec4 centerSize;     // Size and Center
layout(location = 2) in vec4 color;

// Output data; will be interpolated for each fragment.
out vec2 UV;
out vec4 particlecolor;

uniform vec3 cameraRight;
uniform vec3 cameraUp;
uniform mat4 viewProjection; // MVP, but without M

void main()
{
	float particleSize   = centerSize.w; // because we encoded it this way.
	vec3  particleCenter = centerSize.xyz;

	vec3 vertPos =
		particleCenter
		+ cameraRight  * squareVertices.x * particleSize
		+ cameraUp     * squareVertices.y * particleSize;

	// Output position of the vertex
	gl_Position = viewProjection * vec4(vertPos, 1.0f);

	// UV of the vertex. No special space for this one.
	UV = squareVertices.xy + vec2(0.5, 0.5);
	particlecolor = color;
}

