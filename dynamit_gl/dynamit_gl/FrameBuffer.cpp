#include "pch.h"
#include <GL/glew.h>
#include <iostream>
#include "FrameBuffer.h"
#include "config.h"
#include "Terrain.h"
template class FrameBuffer<Terrain>;

template<class T> FrameBuffer<T>::FrameBuffer(const char* vertexPath, const char* fragmentPath)
	: runner(vertexPath, fragmentPath)
{
	build();
}
template<class T> void FrameBuffer<T>::build()
{
	// framebuffer configuration
	glGenFramebuffers(1, &framebuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// create a color attachment texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, config::windowWidth, config::windowHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);

	// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
	unsigned int rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, config::windowWidth, config::windowHeight); // use a single renderbuffer object for both a depth AND stencil buffer.
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it

	// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

template<class T> void FrameBuffer<T>::drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection)
{
	runner.drawInit(model, view, projection, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
}
template<class T> void FrameBuffer<T>::draw(std::function<void(void)> _drawInit)
{
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	{
		_drawInit();
		runner.draw();
		glBindVertexArray(0); //unbind any vao (really needed??)
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); //disable current framebuffer
}

