#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <iostream>
#include "HeartCyllinder.h"
using namespace std;

HeartCyllinder::HeartCyllinder(int heightsegm, int sectors)
{
	nh = nh > MAXH ? MAXH : (heightsegm < 1 ? 10 : heightsegm);
	ns = sectors > MAXS ? MAXS : (sectors < 3 ? 10 : sectors);
	double hstep = 1.0/nh;
	double astep = -360.0/ns;
	double hcur = 0.0, acur = 0, rcur = 1;

	const double rLim = 1.5;
	double rMax = ns / 2 * 0.1;
	double ratio = rMax / rLim;
	cout << "rMax = "<< rMax<< endl;
	for(int j = 0; j <= ns / 2; j++)
	{
		rcur = j * 0.1 / ratio;
		cout << "rcur = "<< rcur<<"; acur = "<< acur<< endl;
		c[0][j][0] = rcur * cos(RADGRAD * acur);
		c[0][j][1] = rcur * sin(RADGRAD * acur);
		acur += astep;
	}
	acur = 0;
	cout << " ************************ "<<endl;
	for(int j = ns / 2, z = ns / 2; j <= ns ; j++, z--)
	{//break;
		rcur = -z * 0.1 / ratio;
		cout << "rcur = "<< rcur<<"; acur = "<< acur<< endl;
		c[0][j][0] = rcur * cos(RADGRAD * acur);
		c[0][j][1] = -rcur * sin(RADGRAD * acur);
		acur -= astep;
	}
	for(int i = 1; i <= nh; i++) //segments
	{
		for(int j = 0; j < ns; j++) //sectors
		{
			c[i][j][0] = c[0][j][0];
			c[i][j][1] = c[0][j][1];
			c[i][j][2] = hcur;
		}
		hcur += hstep;
	}
}

void HeartCyllinder::draw()
{
	glPushMatrix();
	glTranslated(0.5, 0, 0);
	//glBegin(GL_TRIANGLE_FAN);
	//	glVertex3d(0., 0., 0.);
	//	for(int j = 0; j < ns; j++)
	//		glVertex3dv(c[0][j]);
	//	glVertex3dv(c[0][0]);
	//glEnd();
	for(int i = 0; i < nh; i++) //segments
	{
		//glBegin(GL_TRIANGLE_STRIP);
		glBegin(GL_LINE_STRIP);
		for (int j = 0; j < ns ; j++) //sectors
		{
			glVertex3dv(c[i + 1][j]);
			glVertex3dv(c[i][j]);
		}
		glVertex3dv(c[i+1][0]);
		glVertex3dv(c[i][0]);
		glEnd();
	}
	glPopMatrix();
}
