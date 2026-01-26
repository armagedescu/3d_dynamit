#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <iostream>
#include "HeartCone.h"
using namespace std;
HeartCone::HeartCone(int heightsegm, int sectors)
{
	sectors += sectors & 1;
	nh = nh > MAXH ? MAXH : (heightsegm < 1 ? 10 : heightsegm);
	ns = sectors > MAXS ? MAXS : (sectors < 3 ? 10 : sectors);
	points.setSizes(nh, ns);
	calculate3();
}
void HeartCone::calculate1()
{
	const double rLim = 1.5;
	double rMax = ns / 2 * 0.1;
	double ratio = rMax / rLim;

	double hstep = 1.0/nh;
	double astep = -360.0/ns;
	double hcur = 0.0, acur, rcur;
	for(int i = 0; i < nh; i++)
	{
		hcur += hstep;
		acur = 0.0;
		//int j;
		for(int j = 0; j <= ns / 2; j++)
		{
			rcur = hcur * j * 0.1 / ratio;
			if (i == nh - 1) cout << "rcur = "<< rcur<<"; acur = "<< acur<< endl;
			points[i][j][0] ( rcur * cos(RADGRAD * acur) + hcur / 2 );
			points[i][j][1] ( rcur * sin(RADGRAD * acur) );
			points[i][j][2] ( hcur );
			acur += astep;
		}
		if (i == nh - 1) cout << " ************************ "<<endl;
		for(int  j = ns / 2, z = ns / 2; j <= ns ; j++, z--)
		{
			acur -= astep;
			rcur = hcur * z * 0.1 / ratio;
			if (i == nh - 1) cout << "rcur = "<< rcur<<"; acur = "<< acur<< endl;
			points[i][j][0] ( rcur * cos(RADGRAD * acur) + hcur / 2 );
			points[i][j][1] ( rcur * -sin(RADGRAD * acur) );
			points[i][j][2] ( hcur );
		}
	}
}

void HeartCone::calculate2()
{
	const double rLim = 1.5;
	double rMax = ns / 2 * 0.1;
	double ratio = rMax / rLim;

	double hstep = 1.0/nh;
	double astep = -360.0/ns;
	double hcur = 0.0, acur, rcur;
	for(int i = 0; i < nh; i++)
	{
		hcur += hstep;
		acur = 0.0;
		for(int j = 0, jp = ns / 2, jm = ns / 2; j <= ns / 2; j++, jp--, jm++)
		{
			rcur = hcur * j * 0.1 / ratio;
			//if (i == nh - 1) cout << "rcur = "<< rcur<<"; acur = "<< acur<< endl;
			points[i][j][0] ( rcur * cos(RADGRAD * acur) + hcur / 2 );
			points[i][j][1] ( rcur * sin(RADGRAD * acur) );
			points[i][j][2] ( hcur );
			acur += astep;
		}
		//if (i == nh - 1) cout << " ************************ "<<endl;
		for(int  j = ns / 2, z = ns / 2; j < ns ; j++, z--)
		{
			//if (i == nh - 1) cout << "rcur = "<< rcur<<"; acur = "<< acur<< endl;
			points[i][j][0] (  points[i][z][0] );
			points[i][j][1] ( -points[i][z][1] );
			points[i][j][2] (  points[i][z][2] );
		}
	}
}
void HeartCone::calculate3()
{
	//Raza va creste de la 0 la 1.5 pentru ns/2 sectoare si apoi va scade de la 1.5 la 0 pana la ns
	const double rmax = 1.5;
	const double ratio = rmax / (ns / 2); // rata de modificare a unghiului pentru fiecare sector
	//Inaltimea va fi egala cu 1
	const double h = 1.0;
	const double hstep =    1.0 / nh; //Grosimea unui strat
	const double pistep = -2 * M_PI / ns;


	for(int j = 0; j <= ns / 2; j++) //pana la 180 de grade inclusiv, calculat in numar de sectoare
	{
		double rcur = (double)j * ratio; // Curba lui Arhimede. Raza curenta: unghiul inmultit cu un coeficient
		double picur = pistep * j; // Unghiul curent in radiani
		points[nh - 1][j][0] (rcur * cos(picur));// + 0.5);// mutam figura cu 0.5 unitati in directia axei X
		points[nh - 1][j][1] (rcur * sin(picur));
		points[nh - 1][j][2] (h); // h = 1
	}
	cout<< "***************************************************************"<< endl;
	for(int j = ns / 2 + 1, z = ns / 2 - 1; j < ns ; j++, z--) //pana la 180 de grade exclusiv, calculat in numar de sectoare
	{
		points[nh - 1][j][0] ( points[nh - 1][z][0]);
		points[nh - 1][j][1] (-points[nh - 1][z][1]);
		points[nh - 1][j][2] ( points[nh - 1][z][2]); // h = 1
	}


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

}
void HeartCone::draw()
{
	glPushMatrix();
	double t[16] = {
		   1, 0, 0, 0,
		   0, 1, 0, 0,
		 0.5, 0, 1, 0,
		   0, 0, 0, 1
	};
	glMultMatrixd (t);
	//double s[16] = {
	//	   1, 0, 0, 0,
	//	   0, 1, 0, 0,
	//	   0, 0, 1, 0,
	//	   0, 0, 0, 1
	//};
	//glMultMatrixd (t);
	glScaled(1., 0.9, 1.);
    glBegin(GL_TRIANGLES);
    for (int j = 0; j < ns; j++)
    {
		glVertex3d(0., 0., 0.);
        glVertex3dv(points[0][j]);
        glVertex3dv(points[0][(j + 1) % ns]);
    }


    for(int i = 0; i < nh - 1; i++)
    {
        for (int j = 0; j < ns; j++)
        {
            glVertex3dv((double*)points[i][j]);
            glVertex3dv((double*)points[i + 1][j]);
            glVertex3dv((double*)points[i + 1][(j + 1) % ns]);

            glVertex3dv((double*)points[i][j]);
            glVertex3dv((double*)points[i + 1][(j + 1) % ns]);
            glVertex3dv((double*)points[i][(j + 1) % ns]);
        }
    }
    glEnd();
	glPopMatrix();


}
void HeartCone::draw1()
{
	glBegin(GL_TRIANGLE_FAN);
		glVertex3d(0., 0., 0.);
		for(int j = 0; j < ns; j++)
			glVertex3dv(points[0][j]);
		glVertex3dv(points[0][0]);
	glEnd();

	for(int i = 0; i < nh - 1; i++)
	{
		//glBegin(GL_TRIANGLE_STRIP);
		glBegin(GL_LINE_STRIP);
		for (int j = 0; j < ns; j++)
		{
			glVertex3dv(points[i + 1][j]);
			glVertex3dv(points[i][j]);
		}
		glVertex3dv(points[i+1][0]);
		glVertex3dv(points[i][0]);
		glEnd();
	}

}
void HeartCone::draw2()
{

	glBegin(GL_TRIANGLE_FAN);
		glVertex3d(0., 0., 0.);
		for(int j = 0; j < ns; j++)
			glVertex3dv(points[0][j]);
		glVertex3dv(points[0][0]);
	glEnd();

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	//glPolygonMode(GL_FRONT, GL_FILL);
	glColor3d(1., 0., 1.);

	for(int i = 0; i < nh - 1; i++)
	{
		glBegin(GL_TRIANGLE_STRIP);
			for (int j = 0; j < ns; j++)
			{
				glVertex3dv(points[i + 1][j]);
				glVertex3dv(points[i][j]);
			}
			glVertex3dv(points[i+1][0]);
			glVertex3dv(points[i][0]);
		glEnd();


	}
	glCullFace(GL_BACK);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3d(0., 0., 1.);
    
	for(int i = 0; i < nh - 1; i++)
	{
    
		glBegin(GL_TRIANGLE_STRIP);
		//glBegin(GL_TRIANGLES);
		//glBegin(GL_POLYGON);
			for (int j = 0; j < ns; j++)
			{
				glVertex3dv(points[i + 1][j]);
				glVertex3dv(points[i][j]);
			}
			glVertex3dv(points[i+1][0]);
			glVertex3dv(points[i][0]);
		glEnd();
	}
	glDisable(GL_CULL_FACE);
}
