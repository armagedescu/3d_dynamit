#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glaux.h>
#include <stdio.h>

#include "auxFunctions.h"
#include "glFunctions.h"


int auxMain(int argc, char** argv)
{
	// Stabilim coordonatele ferestrei pe ecran
	// coltul stânga sus (0,0)
	// lãþimea ºi înãlþimea - 500
	//1
	auxInitPosition( 0, 0, 500, 500);
	// Stabilim parametrii ale contextului OpenGL
	auxInitDisplayMode( AUX_RGB | AUX_DEPTH | AUX_DOUBLE );
	// Cream o fereastrã pe ecran
	auxInitWindow((LPCWSTR)("OpenGL"));

	// Aceastã fereastrã va primi mesajele
	// de la tastatura, mouse-ul, timer-ul ºi orice alte mesaje.
	// În timp ce nu vine nici un mesaj
	// va fi apelatã ciclic funcþia display.
	// Astfel se creeazã animaþia.
	// Dacã avem nevoie de o imagine staticã
	// comentãm urmãtoarea linie
	auxIdleFunc(auxDisplay);

	// In cazul când se schimbã dimensiunile ferestrei
	// vor fi primit mesajul corespunzãtor.
	// În Windows - acesta este WM_SIZE.
	// Indicãm funcþia resize ca funcþie care va fi
	// apelatã fiecare datã când se schimbã dimensiunile
	// ferestrei
	auxReshapeFunc(resize);
	// Mai departe stabilim un set de teste si parametri

	// Testul de transparentã, adicã va fi luat în consideraþie
	// al patrulea parametru în glColor
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

	auxMainLoop(auxDisplay);
	return 0;
}
