#include "pch.h"
#include "FrameBufferDepthMap.h"
#include "CubeScene.h"
#include <iostream>

template class FrameBufferDepthMap<CubeScene>;

template<class T>
FrameBufferDepthMap<T>::FrameBufferDepthMap(T& shape)
{
}

template<class T> FrameBufferDepthMap<T>::FrameBufferDepthMap(const char* vertexPath, const char* fragmentPath)
	: runner(vertexPath, fragmentPath)
{
	build();
}

template<class T> void FrameBufferDepthMap<T>::build()
{
	// configure depth map FBO
	glGenFramebuffers(1, &framebuffer);
	// create depth texture
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, texture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

template<class T> void FrameBufferDepthMap<T>::drawInit(glm::mat4& lightSpaceMatrix)
{
	runner.drawInit(lightSpaceMatrix);
}
template<class T> void FrameBufferDepthMap<T>::draw(std::function<void(void)> _drawInit)
{
	glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

	{
		_drawInit();
		runner.draw();
		glBindVertexArray(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0); //disable current framebuffer
}
