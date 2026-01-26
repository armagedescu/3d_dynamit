#include <windows.h>
#include <GL/gl.h>
#include <GL/glut.h>
#include <stdio.h>
#include <iostream>
#include "externals.h"
#include "lab1.h"
using namespace std;
void glutDrawCones()
{
	// Con X
	glColor3d(1,0,0);
	glPushMatrix();
	   glTranslated(5.3f, 0.0f, 0.0f);
	   glRotated(90, 0.0f, 1.0f, 0.0f);
	   glutSolidCone(0.1f, 0.2f, 15, 10);
	glPopMatrix();
	// Con Y
	glColor3d(0,1,0);
	glPushMatrix();
	   glTranslated(0.0f, 5.3f, 0.0f);
	   glRotated(-90, 1.0f, 0.0f, 0.0f);
	   glutSolidCone(0.1f, 0.2f, 15, 10);
	glPopMatrix();
	// Con Z
	glColor3d(0,0,1);
	glPushMatrix();
	   glTranslated(0.0f, 0.0f, 5.3f);
	   glutSolidCone(0.1f, 0.2f, 15, 10);
	glPopMatrix();
}
void glutDrawAll()
{
	drawCentralPoint();
	drawAxis();
	glutDrawCones();
}
void GLUTCALLBACK glutDisplay(void)
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	// glClear( GL_COLOR_BUFFER_BIT );
	// glClear( GL_DEPTH_BUFFER_BIT );
	glPushMatrix();
	spacePrepare();
	glutDrawAll();

	//VVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVVV
	// Aici vom adãuga fragmente de cod pentru diferite
	// exemple, pe parcursul studierii funcþiilor OpenGL
	//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	lab1GlDisplay();

	glPopMatrix();
	glutSwapBuffers();
}