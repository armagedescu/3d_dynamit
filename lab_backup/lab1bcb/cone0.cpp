#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>
#include <iostream>
#include "cone0.h"
using namespace std;

cone0::cone0(int heightsegm, int sectors)
{
	sectors += 4 - sectors % 4;
	nh = nh > MAXH ? MAXH : (heightsegm < 1 ? 10 : heightsegm);
	ns = sectors > MAXS4 ? MAXS4 : (sectors < 3 ? MINS4 : sectors);

	calculate3();


}
void cone0::calculate1()
{
	double hstep = 1.0/nh;
	double astep = -360.0/ns;
	double hcur = 0.0, acur, rcur;
	for(int i = 0; i < nh; i++)
	{
		hcur += hstep;
		acur = 0.0;
		rcur = hcur;
		for(int j = 0; j < ns; j++)
		{
			c[i][j][0] = rcur * cos(RADGRAD * acur);
			c[i][j][1] = rcur * sin(RADGRAD * acur);
			c[i][j][2] = hcur;
			acur += astep;
		}
	}
}

void cone0::calculate2()
{
	double hstep = 1.0 / nh;
	double astep = -360.0/ns;
	double acur, rcur;

	for(int j = 0; j < ns ; j++)
	{
		acur = astep * j;
		c[nh - 1][j][0] = cos(RADGRAD * acur);
		c[nh - 1][j][1] = sin(RADGRAD * acur);
		c[nh - 1][j][2] = 1;
	}

	for(int i = nh - 2; i >= 0; i--)
	{
		rcur = hstep * (i + 1);
		for(int j = 0; j < ns; j++)
		{
			acur = astep * j;
			c[i][j][0] = rcur * c[nh - 1][j][0];
			c[i][j][1] = rcur * c[nh - 1][j][1];
			c[i][j][2] = rcur;
		}
	}
}

void cone0::calculate3()
{
	double hstep =    1.0 / nh;
	double astep = -360.0 / ns;
	double acur, rcur;

	for(int j = 0; j <= ns / 4; j++)
	{
		acur = astep * j;
		c[nh - 1][j][0] = cos(RADGRAD * acur);
		c[nh - 1][j][1] = sin(RADGRAD * acur);
		c[nh - 1][j][2] = 1;
		//cout<< c[nh - 1][j][0]<< ";"<< c[nh - 1][j][1]<< endl;
	}
	//cout<< "***************************" << endl;
	for(int j = ns / 4 + 1, jt = ns / 4 - 1; j <= ns / 2; j++, jt--)
	{
		acur = astep * j;
		c[nh - 1][j][0] = -c[nh - 1][jt][0];
		c[nh - 1][j][1] =  c[nh - 1][jt][1];
		c[nh - 1][j][2] = 1;
		//cout<< c[nh - 1][j][0]<< ";"<< c[nh - 1][j][1]<< endl;
	}
	//cout<< "***************************" << endl;
	for(int j = ns  / 2 + 1, ji = ns / 2 - 1; j < ns; j++, ji--)
	{
		c[nh - 1][j][0] =  c[nh - 1][ji][0];
		c[nh - 1][j][1] = -c[nh - 1][ji][1];
		c[nh - 1][j][2] = 1;
		//cout<< c[nh - 1][j][0]<< ";"<< c[nh - 1][j][1]<< endl;
	}

	for(int i = nh - 2; i >= 0; i--)
	{
		rcur = hstep * (i + 1);
		for(int j = 0; j < ns; j++)
		{
			acur = astep * j;
			c[i][j][0] = rcur * c[nh - 1][j][0];
			c[i][j][1] = rcur * c[nh - 1][j][1];
			c[i][j][2] = rcur;
		}
	}
}

void cone0::draw()
{
    glBegin(GL_TRIANGLES);
    for (int j = 0; j < ns; j++)
    {
        glVertex3d(0., 0., 0.);
        glVertex3dv(c[0][j]);
        glVertex3dv(c[0][(j + 1) % ns]);
    }


    for(int i = 0; i < nh - 1; i++)
    {
        for (int j = 0; j < ns; j++)
        {
            glVertex3dv(c[i][j]);
            glVertex3dv(c[i + 1][j]);
            glVertex3dv(c[i + 1][(j + 1) % ns]);

            glVertex3dv(c[i][j]);
            glVertex3dv(c[i + 1][(j + 1) % ns]);
            glVertex3dv(c[i][(j + 1) % ns]);
            
        }
    }
    glEnd();
}

void cone0::draw1()
{
	glBegin(GL_TRIANGLE_FAN);
		glVertex3d(0., 0., 0.);
		for(int j = 0; j < ns; j++)
			glVertex3dv(c[0][j]);
		glVertex3dv(c[0][0]);
	glEnd();

	for(int i = 0; i < nh - 1; i++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for (int j = 0; j < ns; j++)
		{
			glVertex3dv(c[i + 1][j]);
			glVertex3dv(c[i][j]);
		}
		glVertex3dv(c[i+1][0]);
		glVertex3dv(c[i][0]);
		glEnd();
	}
}
