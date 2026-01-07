#include "pch.h"
#include <GL/glew.h>
#include "GoogleMapTerrainIndexed.h"
#include "BitmapReader.h"  // Bitmaps
#include "geometry.h"
#include <glm/glm.hpp> //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp> //convert glm types to opengl types
#include "config.h"
#include <iomanip>

const wchar_t* GoogleMapTerrainIndexed::defTerrainImgPath = L"bitmaps/heightmap.bmp";

GoogleMapTerrainIndexed::GoogleMapTerrainIndexed(const wchar_t* heigthsMapPath)
	: GoogleMapTerrainIndexed(heigthsMapPath, "shaders/googleMapTerrain.vs", "shaders/googleMapTerrain.fs")
{
}
GoogleMapTerrainIndexed::GoogleMapTerrainIndexed(const char* vertexPath, const char* fragmentPath)
	: GoogleMapTerrainIndexed(defTerrainImgPath, vertexPath, fragmentPath)
{}

GoogleMapTerrainIndexed::GoogleMapTerrainIndexed(const wchar_t* heigthsMapPath, const char* vertexPath, const char* fragmentPath)
	: terrainImgPath(heigthsMapPath),
	pos(0.0f, 0.0f, 0.0f),
	Shape(vertexPath, fragmentPath)
{
	build();
}

int GoogleMapTerrainIndexed::fillHeightMapBuffer(float size, float h)
{
	const float bottom = -0.7, maxheight = 0.2;
	if (!vertexes.empty()) return 0;
	std::vector<std::vector<float>> heights;
	int bpr = HeigthMapFromImg(terrainImgPath, heights);
	if (bpr == -1) return -1;

	unsigned int length, width;
	length = heights.size()    - 1; // number of lines is height
    width  = heights[0].size() - 1; // number of points in a single line . < ..... > . width
	//length = 70;
	//width  = 70;

	int coordsCount = length * width * 3; //no triangles, 3 points per vertex, the triangles are on indexes
	int vertsize = coordsCount * 2;       //3 points per vertex + 3 points per normal
	vertexes.reserve (vertsize * 2ll);    //two coats, vertices + norms
	vertexes.resize  (vertsize);    //size for one coat, vertices + norms
	float* viter = vertexes.data(); //vertex iterator

	float rfi = 2.0f / length, rfj = 2.0f / width;

	//std::cout << std::fixed << std::setprecision(3) << std::showpos;
	//using std::cout;
	//using std::endl;

	for (int i = 0; i < length; i++)
	{
		for (int j = 0; j < width; j++)
		{
			float  x,  y,  z;
			float x1, y1, z1;
			float x2, y2, z2;
			float nx = 0, ny = 0, nz = 0;
			x  = i;     y  = heights[i][j];               z  = j;
			x1 = i;     y1 = heights[i][j + 1ll];         z1 = j + 1;
			x2 = i + 1, y2 = heights[i + 1ll][j + 1ll];   z2 = j + 1;
			resize3( x,  y,  z, rfi, maxheight, rfj); offset3( x,  y,  z, -1.f, bottom, -1.f);
			resize3(x1, y1, z1, rfi, maxheight, rfj); offset3(x1, y1, z1, -1.f, bottom, -1.f);
			resize3(x2, y2, z2, rfi, maxheight, rfj); offset3(x2, y2, z2, -1.f, bottom, -1.f);
			norm3nz<float>(nx, ny, nz, x1 - x, y1 - y, z1 - z, x2 - x, y2 - y, z2 - z, 2);
			viter[0] =  x; viter[1] =  y; viter[2] =  z;
			viter[3] = nx; viter[4] = ny; viter[5] = nz;
			viter += stridesize;
		}
	}

	//indexes
	int rectanglesCount = (length - 1) * (width - 1);
	trianglesCount = rectanglesCount * 2;    //two triangles per rectangle
	int indicescount = trianglesCount * 3ll; //three edges (indices) per triangle
	indexes.reserve(indicescount * 2ll); //two sides
	indexes.resize (indicescount); //one side
	int* iiter = indexes.data();   //index iterator
	for (int i = 0; i < length - 1; i++)
	{
		for (int j = 0; j < width - 1; j++)
		{
			iiter[0] =      i  * width + j;
			iiter[1] =      i  * width + (j + 1);
			iiter[2] = (i + 1) * width + (j + 1);
			iiter += 3;
			iiter[0] =      i  * width  + j;
			iiter[1] = (i + 1) * width  + (j + 1);
			iiter[2] = (i + 1) * width  + j;
			iiter += 3;
		}
	}

	//////double coating it
	if (!doubleCoated) return 0;
	vertexes.resize(vertexes.size() * 2);
	indexes.resize(indexes.size() * 2);

	for (int i = 0; i < length; i++)
	{
		for (int j = 0; j < width; j++)
		{
			float  x,  y,  z;
			float x1, y1, z1;
			float x2, y2, z2;
			float nx = 0, ny = 0, nz = 0;
			x  = i;     y  = heights[i][j];               z  = j;
			x2 = i;     y2 = heights[i][j + 1ll];         z2 = j + 1;
			x1 = i + 1, y1 = heights[i + 1ll][j + 1ll];   z1 = j + 1;
			resize3( x,  y,  z, rfi, maxheight, rfj); offset3(x, y, z, -1.f, bottom, -1.f);
			resize3(x1, y1, z1, rfi, maxheight, rfj); offset3(x1, y1, z1, -1.f, bottom, -1.f);
			resize3(x2, y2, z2, rfi, maxheight, rfj); offset3(x2, y2, z2, -1.f, bottom, -1.f);
			norm3nz<float>(nx, ny, nz, x1 - x, y1 - y, z1 - z, x2 - x, y2 - y, z2 - z, 2);
			viter[0] =  x; viter[1] =  y; viter[2] =  z;
			viter[3] = nx; viter[4] = ny; viter[5] = nz;
			viter += stridesize;
		}
	}

	for (int i = length, i0 = 0; i0 < length - 1; i++, i0++)
	{
		for (int j = 0; j < width - 1; j++)
		{
			//use iiter instead of iter
			iiter[0] =      i  * width +  j;
			iiter[2] =      i  * width + (j + 1);
			iiter[1] = (i + 1) * width + (j + 1);
			iiter += 3;
			iiter[0] =      i  * width +  j;
			iiter[2] = (i + 1) * width + (j + 1);
			iiter[1] = (i + 1) * width +  j;
			iiter += 3;
		}
	}
	return 0;
}

void GoogleMapTerrainIndexed::build()
{
	fillHeightMapBuffer(1, 1);
	vertexes.shrink_to_fit(); //will not have effect if double coated
	indexes.shrink_to_fit();  //will not have effect if double coated
	float* pv = vertexes.data();
	int *pi = indexes.data();

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vertexesVbo;
	glGenBuffers(1, &vertexesVbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexesVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexes.size(), vertexes.data(), GL_STATIC_DRAW);

	//bind ebo data
	unsigned int ebo;
	glGenBuffers(1, &ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * indexes.size(), indexes.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(vertLocation, 3, GL_FLOAT, GL_FALSE, stridesize * sizeof(float), 0);
	glEnableVertexAttribArray(vertLocation);

	glVertexAttribPointer(normLocation, 3, GL_FLOAT, GL_FALSE, stridesize * sizeof(float), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(normLocation);
	// color attribute
	glBindVertexArray(0);

	modelLocationId      = glGetUniformLocation(*this, "model");
	viewLocationId       = glGetUniformLocation(*this, "view");
	projectionLocationId = glGetUniformLocation(*this, "projection");

}

void GoogleMapTerrainIndexed::drawInit()
{
	using config::camera;
	using config::windowWidth;
	using config::windowHeight;
	glm::mat4 model = glm::mat4(1.0);
	model = glm::translate(model, pos);
	model = glm::rotate(model, (float)(glfwGetTime()), glm::vec3(0.0f, 1.0f, 0.0f));
	glm::mat4 view = camera.view();
	glm::mat4 projection = glm::mat4(1.0f);
	projection = glm::perspective(glm::radians(camera.zoom), (float)windowWidth / windowHeight, 0.1f, 100.0f);

	drawInit(model, view, projection, glm::vec4(1, 0, 0, 1));
}
void GoogleMapTerrainIndexed::drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection, const glm::vec4& color)
{
	glUseProgram(*this);
	glUniformMatrix4fv(modelLocationId,      1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLocationId,       1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLocationId, 1, GL_FALSE, glm::value_ptr(projection));

	glVertexAttrib4fv(vertColorLocation, glm::value_ptr(color));

}
void GoogleMapTerrainIndexed::draw()
{
	glUseProgram(*this);
	glBindVertexArray(vao);
	glDrawElements(GL_TRIANGLES, indexes.size(), GL_UNSIGNED_INT, 0);
}