#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <stdlib.h>
#include <GL/glut.h>
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

static void display(void)
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

static void reshape(int w, int h)
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



int bezierGlutMain(int argc, char** argv)
{
   glutInit(&argc, argv);


   glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);

   glutInitWindowSize (500, 500);
   glutInitWindowPosition (100, 100);
   glutCreateWindow (argv[0]);
   init ();
   glutDisplayFunc(display);
   glutReshapeFunc(reshape);
   glutMainLoop();
   return 0;
}
#include <gl/GLAux.h>
void CALLBACK auxBezierDisplay(void){display();}
void CALLBACK auxBezierReshape(GLsizei x, GLsizei y){reshape(x, y);}
int bezierAuxMain(int argc, char** argv)
{
   auxInitDisplayMode( AUX_RGB | AUX_SINGLE);
   auxInitPosition( 100, 100, 500, 500);
   auxInitWindow((LPCWSTR)("OpenGL"));
   auxIdleFunc(auxBezierDisplay);
   auxReshapeFunc(auxBezierReshape);
   init ();
   auxMainLoop(auxBezierDisplay);
   return 0;
}