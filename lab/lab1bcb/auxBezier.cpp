#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <gl/GLAux.h>
#include <iostream>
using namespace std;

static GLfloat ctrlpoints[4][3] = {
        { -4.0, -4.0, 0.0}, { -2.0, 4.0, 0.0}, 
        {  2.0, -4.0, 0.0}, {  4.0, 4.0, 0.0}};

static void init(void)
{
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glShadeModel(GL_FLAT);
   glMap1f(GL_MAP1_VERTEX_3, 0.0, 1.0, 3, 4, &ctrlpoints[0][0]);
   glEnable(GL_MAP1_VERTEX_3);
}

void CALLBACK auxBezierDisplay(void)
{
   int i;
   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(1.0, 0.0, 0.0);
   glBegin(GL_LINE_STRIP);
      for (i = 0; i <= 30; i++) 
		glEvalCoord1f((GLfloat) i/30.0);
   glEnd();
   /* The following code displays the control points as dots. */
   glPointSize(5.0);
   glColor3f(1.0, 1.0, 0.0);
   glBegin(GL_POINTS);
      for (i = 0; i < 4; i++) 
         glVertex3fv(&ctrlpoints[i][0]);
   glEnd();
   glFlush();
}

void CALLBACK auxBezierReshape(int w, int h)
{
   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   if (w <= h)
      glOrtho(-5.0, 5.0, -5.0*(GLfloat)h/(GLfloat)w, 
               5.0*(GLfloat)h/(GLfloat)w, -5.0, 5.0);
   else
      glOrtho(-5.0*(GLfloat)w/(GLfloat)h, 
               5.0*(GLfloat)w/(GLfloat)h, -5.0, 5.0, -5.0, 5.0);
   glMatrixMode(GL_MODELVIEW);
   glLoadIdentity();
}
static void init2(void)
{
   glClearColor(0.0, 0.0, 0.0, 0.0);
   glShadeModel(GL_FLAT);
}
void CALLBACK auxBezierDisplay2(void)
{
   int i;
   glClear(GL_COLOR_BUFFER_BIT);
   glColor3f(1.0, 0.0, 0.0);
   glBegin(GL_LINE_STRIP);
	  for (i = 0; i <= 30; i++)
	  {
        double t = (double)i / 30;
		double x1t = ctrlpoints[0][0] * (1 - t) + ctrlpoints[1][0] * t;
		double y1t = ctrlpoints[0][1] * (1 - t) + ctrlpoints[1][1] * t;

		double x2t = ctrlpoints[1][0] * (1 - t) + ctrlpoints[2][0] * t;
		double y2t = ctrlpoints[1][1] * (1 - t) + ctrlpoints[2][1] * t;
		double x1ct = x1t * (1 - t) +  x2t * t;
		double y1ct = y1t * (1 - t) +  y2t * t;
		
		double x3t = ctrlpoints[2][0] * (1 - t) + ctrlpoints[3][0] * t;
		double y3t = ctrlpoints[2][1] * (1 - t) + ctrlpoints[3][1] * t;
		double x2ct = x2t * (1 - t) +  x3t * t;
		double y2ct = y2t * (1 - t) +  y3t * t;

		double xct = x1ct * (1 - t) +  x2ct * t;
		double yct = y1ct * (1 - t) +  y2ct * t;
		
		double x3[3] = {xct,yct,0};
		glVertex3dv(x3);
	  }

   glEnd();
   /* The following code displays the control points as dots. */
   glPointSize(5.0);
   glColor3f(1.0, 1.0, 0.0);
   glBegin(GL_POINTS);
      for (i = 0; i < sizeof (ctrlpoints) / sizeof (GLfloat[3]); i++) 
         glVertex3fv(ctrlpoints[i]);
   glEnd();
   glFlush();
}

int bezierAuxMain(int argc, char** argv)
{
   auxInitDisplayMode( AUX_RGB | AUX_SINGLE);
   auxInitPosition( 100, 100, 500, 500);
   auxInitWindow((LPCWSTR)("OpenGL"));
   //auxIdleFunc(auxBezierDisplay);
   //auxReshapeFunc(auxBezierReshape);
   //init ();
   //auxMainLoop(auxBezierDisplay);
   auxIdleFunc(auxBezierDisplay2);
   auxReshapeFunc(auxBezierReshape);
   init2 ();
   auxMainLoop(auxBezierDisplay2);
   return 0;
}