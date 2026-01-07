#include "pch.h"
#define STB_IMAGE_IMPLEMENTATION //makes sure stb doesn't go trying to compile the rest of the library

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <iostream>
#include <string>
#include "TextureLoader.h"
#include "util.h"


unsigned int LoadTexture(const char* path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char*        data = stbi_load(path,     &width, &height, &nrComponents,     0);
	scope_guard freeTextureData([data]() { if (data) stbi_image_free(data); std::cout << "texture freed" << std::endl; });
	if (!data)
		return textureID;

	GLenum format = GL_RGB;
	switch (nrComponents)
	{
	case 1: format = GL_RED;  break;
	case 3: format = GL_RGB;  break;
	case 4: format = GL_RGBA; break;
	}

	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);


    return textureID;
}


unsigned int LoadTexture(const char* fileName, int RGBType)
{
	//generate texture in gfx card and get its ID
	unsigned int textureID1 = 0;
	glGenTextures(1, &textureID1);
	int width, height, numberOfChannels;
	unsigned char* imageData = stbi_load(fileName, &width, &height, &numberOfChannels, 0);
	scope_guard freeTextureData([imageData]() {if (imageData) stbi_image_free(imageData);});
	// TODO: report failure and end program
	if (!imageData) return textureID1;

	//bind this texture to make it the current one
	glBindTexture(GL_TEXTURE_2D, textureID1);

	//WRAPPING OPTIONS
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);        //s = horizonal axis
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE); //t = vertical axis
	//FILTERING OPTIONS
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //on shrink
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);//on stretch

	//these vars get filled in when image is loaded

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, RGBType, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);

	return textureID1;
}