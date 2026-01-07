#include "pch.h"
#include <GL/glew.h>
#include "Terrain.h"
#include "BitmapReader.h"  // Bitmaps
#include "geometry.h"
#include <glm/glm.hpp> //basic glm math functions
#include <glm/gtc/matrix_transform.hpp> //matrix functions
#include <glm/gtc/type_ptr.hpp> //convert glm types to opengl types
#include "config.h"
#include <iomanip>
using std::cout;
using std::endl;
const wchar_t* Terrain::defTerrainImgPath = L"bitmaps/heightmap.bmp";

Terrain::Terrain(const wchar_t* heigthsMapPath) :
	Terrain(heigthsMapPath, "shaders/terrain.vs", "shaders/terrain.fs")
{
}
Terrain::Terrain(const char* vertexPath, const char* fragmentPath) :
	Terrain(defTerrainImgPath, vertexPath, fragmentPath)
{}

Terrain::Terrain(const wchar_t* heigthsMapPath, const char* vertexPath, const char* fragmentPath) :
	terrainImgPath (heigthsMapPath),
	pos (0.0f, 0.0f, 0.0f),
	Shape(vertexPath, fragmentPath)
{
	build();
}


int Terrain::fillHeightMapBuffer(float size, float h)
{
	if (!vertexes.empty()) return 0;
	std::vector<std::vector<float>> heights;
	int bpr = HeigthMapFromImg(terrainImgPath, heights);
	if (bpr == -1) return -1;

	unsigned int length, width;
	length = heights.size() - 1; width = heights[0].size() - 1;
	//height = 7;
	//width = 7;

	int rectanglesCount = length * width;
	trianglesCount = rectanglesCount * 2;
	vertexesCount  = trianglesCount  * 3;
	int coordsCount = vertexesCount * 3;
	vertexes.reserve(2ll * 2 * coordsCount);
	vertexes.resize(2ll * coordsCount);
	//float rfi = 2.0f / height, rfj = 2.0f / width;
	float rfi = 2.0f / length, rfj = 2.0f / width;
	float* iter = vertexes.data();

	std::cout << std::fixed << std::setprecision(3) << std::showpos;

	for (int i = 0; i < width; i++) //x = from 0 to width
	{
		for (int j = 0; j < length; j++) //z = from 0 to height, y = height
		{
			float x0, y0, z0;
			float x1, y1, z1;
			float x2, y2, z2;
			float nx, ny, nz;
			x0 = i,     y0 = heights[i]    [j],     z0 = j;
			x1 = i,     y1 = heights[i]    [j + 1], z1 = j + 1;
			x2 = i + 1, y2 = heights[i + 1][j + 1], z2 = j + 1;
			resize3(x0, y0, z0, rfi, 2.f, rfj); offset3(x0, y0, z0, -1.f, -1.f, -1.f);
			resize3(x1, y1, z1, rfi, 2.f, rfj); offset3(x1, y1, z1, -1.f, -1.f, -1.f);
			resize3(x2, y2, z2, rfi, 2.f, rfj); offset3(x2, y2, z2, -1.f, -1.f, -1.f);
			norm3nz<float>(nx, ny, nz, x1 - x0, y1 - y0, z1 - z0, x2 - x0, y2 - y0, z2 - z0, 2);

			//deactivate
			//x0 = 0, y0 = 0, z0 = 0; x1 = 0, y1 = 0, z1 = 0; x2 = 0, y2 = 0, z2 = 0;
			iter[0] = x0; iter[1] = y0; iter[2] = z0;
			iter[3] = nx; iter[4] = ny; iter[5] = nz;
			//coutn<3>(cout, iter); coutn<3>(cout, iter + 3) << endl;

			iter += stridesize;
			iter[0] = x1; iter[1] = y1; iter[2] = z1;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
			iter[0] = x2; iter[1] = y2; iter[2] = z2;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
			//coutn<3>(cout, iter - 3) << endl;

			x0 = i,     y0 = heights[i][j],         z0 = j;
			x1 = i + 1, y1 = heights[i + 1][j + 1], z1 = j + 1;
			x2 = i + 1, y2 = heights[i + 1][j],     z2 = j;
			// hide from GL_CULL_FACE
			//x1 = i,     y1 = heights[i][j + 1],     z1 = j + 1; x2 = i + 1, y2 = heights[i + 1][j + 1], z2 = j + 1;
			resize3(x0, y0, z0, rfi, 2.f, rfj); offset3(x0, y0, z0, -1.f, -1.f, -1.f);
			resize3(x1, y1, z1, rfi, 2.f, rfj); offset3(x1, y1, z1, -1.f, -1.f, -1.f);
			resize3(x2, y2, z2, rfi, 2.f, rfj); offset3(x2, y2, z2, -1.f, -1.f, -1.f);
			norm3nz<float>(nx, ny, nz, x1 - x0, y1 - y0, z1 - z0, x2 - x0, y2 - y0, z2 - z0, 2);

			//deactivate:
			//x0 = 0, y0 = 0, z0 = 0; x1 = 0, y1 = 0, z1 = 0; x2 = 0, y2 = 0, z2 = 0;
			iter[0] = x0; iter[1] = y0; iter[2] = z0;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
			iter[0] = x1; iter[1] = y1; iter[2] = z1;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
			iter[0] = x2; iter[1] = y2; iter[2] = z2;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
		}
	}

	////double coating it
	if (!doubleCoated) return 0;
	vertexes.resize(vertexes.size() * 2);
	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < length; j++)
		{
			float x0, y0, z0;
			float x1, y1, z1;
			float x2, y2, z2;
			float nx, ny, nz;
			x0 = i,     y0 = heights [i][j],         z0 = j;
			x2 = i,     y2 = heights [i][j + 1],     z2 = j + 1;
			x1 = i + 1, y1 = heights [i + 1][j + 1], z1 = j + 1;
			resize3(x0, y0, z0, rfi, 2.f, rfj); offset3(x0, y0, z0, -1.f, -1.f, -1.f);
			resize3(x1, y1, z1, rfi, 2.f, rfj); offset3(x1, y1, z1, -1.f, -1.f, -1.f);
			resize3(x2, y2, z2, rfi, 2.f, rfj); offset3(x2, y2, z2, -1.f, -1.f, -1.f);
			norm3nz(nx, ny, nz, x1 - x0, y1 - y0, z1 - z0, x2 - x0, y2 - y0, z2 - z0);
			//deactivate
			//x0 = 0, y0 = 0, z0 = 0; x1 = 0, y1 = 0, z1 = 0; x2 = 0, y2 = 0, z2 = 0;
			iter[0] = x0; iter[1] = y0; iter[2] = z0;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
			iter[0] = x1; iter[1] = y1; iter[2] = z1;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
			iter[0] = x2; iter[1] = y2; iter[2] = z2;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;

			x0 = i, y0 = heights[i][j], z0 = j;
			x2 = i + 1, y2 = heights[i + 1][j + 1], z2 = j + 1;
			x1 = i + 1, y1 = heights[i + 1][j], z1 = j;
			// hide from GL_CULL_FACE
			//x1 = i,     y1 = heights[i][j + 1],     z1 = j + 1; x2 = i + 1, y2 = heights[i + 1][j + 1], z2 = j + 1;
			resize3(x0, y0, z0, rfi, 2.f, rfj); offset3(x0, y0, z0, -1.f, -1.f, -1.f);
			resize3(x1, y1, z1, rfi, 2.f, rfj); offset3(x1, y1, z1, -1.f, -1.f, -1.f);
			resize3(x2, y2, z2, rfi, 2.f, rfj); offset3(x2, y2, z2, -1.f, -1.f, -1.f);
			norm3nz(nx, ny, nz, x1 - x0, y1 - y0, z1 - z0, x2 - x0, y2 - y0, z2 - z0);

			//deactivate:
			//x0 = 0, y0 = 0, z0 = 0; x1 = 0, y1 = 0, z1 = 0; x2 = 0, y2 = 0, z2 = 0;
			iter[0] = x0; iter[1] = y0; iter[2] = z0;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
			iter[0] = x1; iter[1] = y1; iter[2] = z1;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
			iter[0] = x2; iter[1] = y2; iter[2] = z2;
			iter[3] = nx; iter[4] = ny; iter[5] = nz; iter += stridesize;
		}
	}
	return 0;
}


void Terrain::build()
{
	fillHeightMapBuffer(1, 1);

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	unsigned int vertexesVbo;
	glGenBuffers(1, &vertexesVbo);
	glBindBuffer(GL_ARRAY_BUFFER, vertexesVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertexes.size(), vertexes.data(), GL_STATIC_DRAW);

	const int vertLocation = 0; //location in shader
	glVertexAttribPointer(vertLocation, 3, GL_FLOAT, GL_FALSE, stridesize * sizeof(float), 0);
	glEnableVertexAttribArray(vertLocation);

	const int normLocation = 1; //location in shader
	glVertexAttribPointer(normLocation, 3, GL_FLOAT, GL_FALSE, stridesize * sizeof(float), (const void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(normLocation);
	// color attribute
	glBindVertexArray(0);

	modelLocationId      = glGetUniformLocation(*this, "model");
	viewLocationId       = glGetUniformLocation(*this, "view");
	projectionLocationId = glGetUniformLocation(*this, "projection");

}

void Terrain::drawInit()
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

void Terrain::drawInit(glm::mat4& model, glm::mat4& view, glm::mat4& projection, const glm::vec4& color)
{
	glUseProgram(*this);
	glUniformMatrix4fv(modelLocationId,      1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(viewLocationId,       1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(projectionLocationId, 1, GL_FALSE, glm::value_ptr(projection));

	glVertexAttrib4fv(vertColorLocation, glm::value_ptr(color));
}

void Terrain::draw()
{
	glUseProgram(*this);
	glBindVertexArray(vao);
	glDrawArrays(GL_TRIANGLES, 0, vertexes.size() / 3);
}
