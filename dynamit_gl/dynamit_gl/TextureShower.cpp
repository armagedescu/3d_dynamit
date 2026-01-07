#include "pch.h"
#include "TextureShower.h"

const float TextureShower::quadVertices[] = // vertex attributes for a quad that fills the entire screen in Normalized Device Coordinates.
{
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,

	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};
TextureShower::TextureShower()
	:TextureShower("shaders/framebuffer/terrain.screen.vs", "shaders/framebuffer/terrain.screen.fs")
{
}
TextureShower::TextureShower(const char* vertexPath, const char* fragmentPath)
	: Shape(vertexPath, fragmentPath)
{
	build();
}
void TextureShower::build()
{
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vbo;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

	glUseProgram(*this);
	glUniform1i(glGetUniformLocation(*this, "screenTexture"), 0);

}
void TextureShower::draw()
{
	glUseProgram(*this);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, 6);
}

void TextureShower::drawInit(unsigned int texture)
{
	glUseProgram(*this);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);	// use the color attachment texture as the texture of the quad plane
}
void TextureShower::drawInit(unsigned int texture, int near_plane, int far_plane)
{
	glUseProgram(*this);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);	// use the color attachment texture as the texture of the quad plane

	glUniform1f(glGetUniformLocation(*this, "near_plane"), near_plane);
	glUniform1f(glGetUniformLocation(*this, "far_plane"), far_plane);
}
