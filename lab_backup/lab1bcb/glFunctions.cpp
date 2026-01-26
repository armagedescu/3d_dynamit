#include "glfunctions.h"
#include <windows.h>
#include <GL/gl.h>
#include "lab1.h"

void spacePrepare()
{
	glTranslated(0., 0., -6.0);
	glRotated(35. + getDFi(), 1., 0., 0.);
	glRotated(-35. + getDFi(), 0., 1., 0.);
}


void drawCentralPoint()
{
	glPointSize(10.0f);
	glEnable(GL_POINT_SMOOTH);
	   glBegin(GL_POINTS);
	      glColor3d(0,0,0);
	      glVertex3f(0.f,0.f,0.f);
	   glEnd();
	glDisable(GL_POINT_SMOOTH);
}
void drawAxis()
{
	glLineWidth(1.5f);
	glEnable(GL_LINE_SMOOTH);
	glBegin(GL_LINES);
	   // Axa X
	   glColor3d(0.,0.,0.);
	   glVertex3d(-5.5,0.,0.);
	   glColor3d(1.,0.,0.);
	   glVertex3d(5.5,0.,0.);
	   // Axa Y
	   glColor3d(0.,0.,0.);
	   glVertex3d(0.,-5.5,0.);
	   glColor3d(0.,1.,0.);
	   glVertex3d(0.,5.5,0.);
	   // Axa Z
	   glColor3d(0.,0.,0.);
	   glVertex3d(0.,0.,-5.5);
	   glColor3d(0.,0.,1.);
	   glVertex3d(0.,0.,5.5);
	glEnd();
}

void CALLBACK resize(int width, int height)
{
	// Aici se indicã partea ferestrei în care
	// se efectueazã afiºarea de cãtre OpenGL.
	GLuint wp = width<height ? width-20 : height-20;
	glViewport(10, 10, wp, wp);

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	// Stabilim tipul proiecþiei.
	// glOrtho - paralelã
	// glFrustum - perspectivã
	// Parametrii la ambele funcþii sunt identici
	// Ei stabilesc volumul care va fi vizibil
	// peretele stânga - 6.2 unitãþi în stânga
	// dreapta - 6.2 unitãþi în dreapta
	// apoi pereþiile de jos ºi de sus,
	// ºi în sfârºit pereþiile din faþã ºi din spate
	 glOrtho(-6.2,6.2, -6.2,6.2, 2., 12.);
	// glFrustum(-5,5, -5,5, 2,12);

	glMatrixMode( GL_MODELVIEW );
}