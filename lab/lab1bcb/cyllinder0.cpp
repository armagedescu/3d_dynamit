#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include "cyllinder0.h"

cyllinder0::cyllinder0(int heightsegm, int sectors)
{
	nh = nh > MAXH ? MAXH : (heightsegm < 1 ? 10 : heightsegm);
	ns = sectors > MAXS ? MAXS : (sectors < 3 ? 10 : sectors);
	double hstep = 1.0/nh;
	double astep = -360.0/ns;
	double hcur = 0.0, acur = 0, rcur = 1;

	for(int j = 0; j < ns; j++)
	{
		c[0][j][0] = rcur * cos(RADGRAD * acur);
		c[0][j][1] = rcur * sin(RADGRAD * acur);
		acur += astep;
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

void cyllinder0::draw()
{
	//glBegin(GL_TRIANGLE_FAN);
	//	glVertex3d(0., 0., 0.);
	//	for(int j = 0; j < ns; j++)
	//		glVertex3dv(c[0][j]);
	//	glVertex3dv(c[0][0]);
	//glEnd();

	for(int i = 0; i <= nh; i++) //segments
	{
		glBegin(GL_LINE_STRIP);
		for (int j = 0; j < ns; j++) //sectors
		{
			glVertex3dv(c[i + 1][j]);
			glVertex3dv(c[i][j]);
		}
		glVertex3dv(c[i+1][0]);
		glVertex3dv(c[i][0]);
		glEnd();
	}
}
