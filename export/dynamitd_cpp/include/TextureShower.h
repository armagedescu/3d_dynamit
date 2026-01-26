#pragma once
#include "Shape.h"

class TextureShower : public Shape
{

public:
	static const float quadVertices[];
	unsigned int vao;

	TextureShower();
	TextureShower(const char* vertexPath, const char* fragmentPath);

	void draw();
	//void draw(unsigned int texture, int near_plane, int far_plane);
	void drawInit(unsigned int texture, int near_plane, int far_plane);
	void drawInit(unsigned int texture);
	void build();

};
