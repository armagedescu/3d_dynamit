#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>
#include <stdio.h>
#include "externals.h"
#include "glFunctions.h"
#include "glutFunctions.h"

void glutResize(int width, int height) {return resize(width, height);}

int glutMain(int argc, char** argv)
{
   glutInit(&argc, argv);

   //1
   glutInitWindowSize (500, 500);
   glutInitWindowPosition (0, 0);
   glutInitDisplayMode (GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE );
   glutCreateWindow ("OpenGL");
   glutIdleFunc(glutDisplay);
   
   //glutInit ();
   glutDisplayFunc(glutDisplay);
   glutReshapeFunc(glutResize);

    glEnable(GL_ALPHA_TEST);
	// testul de adâncime
	glEnable(GL_DEPTH_TEST);

	// funcþia glColor va stabili
	// proprietãþile materialelor.
	// De aceia nu este nevoie de apelul suplimentar
	// a funcþiei glMaterialfv
	glEnable(GL_COLOR_MATERIAL);

	// Permitem iluminarea
	glEnable(GL_LIGHTING);
	// Aprindem sursa de luminã cu numãrul 0
	glEnable(GL_LIGHT0);
	// Stabilim poziþia sursei de luminã
	float pos[4] = {3,3,3,1};
	float dir[3] = {-1,-1,-1};
	glLightfv(GL_LIGHT0, GL_POSITION, pos);
	glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, dir);

	// Permitem amestecarea culorilor
	// (pentru suprafeþe transparente)
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	// Stabilim culoarea de umplere iniþialã a ferestrei
	glClearColor(1.f, 1.f, 1.f, 1.f);
	// Indicãm cã funcþia display() va fi folositã pentru
	// redesenarea ferestrei.
	// Aceastã funcþie va fi apelatã de fiecare datã când
	// apare necesitatea de a redesena fereastra.
	// De exemplu, când fereastra se desfãºoarã
	// pe întregul ecran (mesajul WM_PAINT in Windows)
   glutMainLoop();
   return 0;
}