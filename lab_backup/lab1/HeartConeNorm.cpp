#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <iostream>
using namespace std;

#include "HeartConeNorm.h"


HeartConeNorm::HeartConeNorm(int heightsegm, int sectors)
{
	sectors += sectors & 1;
	nh = nh      > MAXH ? MAXH : (heightsegm < 1 ? 10 : heightsegm);
	ns = sectors > MAXS ? MAXS : (sectors < 3 ? 10 : sectors);
	points.setdimensions(nh, ns, 3);
	norms.setdimensions(ns, 3);
	calculate();
}

void HeartConeNorm::calculate()
{
	//Raza va creste de la 0 la 1.5 pentru ns/2 sectoare si apoi va scade de la 1.5 la 0 pana la ns
	const double rmax = 1.5;
	const double ratio = rmax / (ns / 2); // rata de modificare a unghiului pentru fiecare sector
	//Inaltimea va fi egala cu 1
	const double h = 1.0;
	const double hstep =       1.0 / nh; //Grosimea unui strat
	const double pistep = 2 * M_PI / ns; //Unghiul unui sector. Creste contra acelor ceasornicului


	// Calculam coordonatele
	int j;
	for(j = 0; j <= ns / 2; j++) //pana la 180 de grade inclusiv, calculat in numar de sectoare
	{
		double dbj = (double)j;
		double rcur = dbj * ratio; // Curba lui Arhimede. Raza curenta: unghiul inmultit cu un coeficient
		double picur = pistep * dbj; // Unghiul curent in radiani
		points[nh - 1][j][0] (rcur * cos(picur));// + 0.5);// mutam figura cu 0.5 unitati in directia axei X, se face in matrita de translare
		points[nh - 1][j][1] (rcur * sin(picur));
		points[nh - 1][j][2] (h); // h = 1

	}
	// Transformam simetric coordonatele in ordine negativa fata de axa Y
	for(int z = ns / 2 - 1; j < ns ; j++, z--) //pana la 180 de grade exclusiv, calculat in numar de sectoare
	{
		points[nh - 1][j][0] ( points[nh - 1][z][0]);
		points[nh - 1][j][1] (-points[nh - 1][z][1]);
		points[nh - 1][j][2] ( points[nh - 1][z][2]); // h = 1

	}

	// Calculam fiecare strat, miscorand stratul precedent proportional cu numarul straturi
	for(int i = nh - 2; i >= 0; i--)
	{
		double hcur = hstep * (i + 1);
		for(int j = 0; j < ns ; j++)
		{
			points[i][j][0] (points[nh - 1][j][0] * hcur);
			points[i][j][1] (points[nh - 1][j][1] * hcur);
			points[i][j][2] (hcur);
		}
	}

	//Calculam vectorii pentru fiecare sector
	for (int j = 0; j < ns; j++)
		norms[j].set (norm(points[0][j], points[1][(j+1)%ns], points[1][j]));

}
//y=(x^2+y^2-1)^(0.5)/arctg(y/x)
void HeartConeNorm::draw()
{
	static GLdouble v[3] = {0.0, 0.0, 0.0};
	glPushMatrix();

	const double s[16] = {
		     1,    0, 0, 0,
		     0, 0.95, 0, 0, //<-- 0.95, aplatizare dupã Y
		   0.5,    0, 1, 0, //<-- 0.5,   deplasare dupã X
		     0,    0, 0, 1
	};
	glMultMatrixd (s);

    glBegin(GL_TRIANGLES);

	for (int j = 0; j < ns; j++)
    {
		glNormal3dv(norms[j]);
		for(int i = nh - 2; i >= 0; i--)
        {
            glVertex3dv((double*)points[i][j]);
            glVertex3dv((double*)points[i + 1][j]);
            glVertex3dv((double*)points[i + 1][(j + 1) % ns]);

            glVertex3dv((double*)points[i][j]);
            glVertex3dv((double*)points[i + 1][(j + 1) % ns]);
            glVertex3dv((double*)points[i][(j + 1) % ns]);
        }
		glVertex3d(0., 0., 0.);
        glVertex3dv(points[0][j]);
        glVertex3dv(points[0][(j + 1) % ns]);
    }

    glEnd();
	glPopMatrix();

}

GLdouble *HeartConeNorm::norm(const GLdouble p1[3], const GLdouble p2[3], const GLdouble p3[3])
{
	static GLdouble p[3];
	p[0] = (p1[1]*(p2[2]-p3[2]) + p2[1]*(p3[2]-p1[2]) + p3[1]*(p1[2]-p2[2]));
	p[1] = (p1[2]*(p2[0]-p3[0]) + p2[2]*(p3[0]-p1[0]) + p3[2]*(p1[0]-p2[0]));
	p[2] = (p1[0]*(p2[1]-p3[1]) + p2[0]*(p3[1]-p1[1]) + p3[0]*(p1[1]-p2[1]));
	return p;
}