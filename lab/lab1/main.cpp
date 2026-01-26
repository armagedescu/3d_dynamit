//#include <windows.h>
//#include <stdio.h>
//#include "MemoryHelpers.h"
//#include "externals.h"
#include <iostream>
int bezierGlutMain(int argc, char** argv);
int bezierAuxMain(int argc, char** argv);
int glutMain(int argc, char** argv);
int auxMain(int argc, char** argv);

using namespace std;
int main(int argc, char** argv)
{
	//return bezierAuxMain( argc,  argv);
	//return bezierGlutMain( argc,  argv);
	return auxMain(argc, argv);
	//return glutMain(argc, argv);
}







	//try
	//{
	//	{
	//		//multiarray<double> z(1, 7);
	//		//for (int j = 0 ; j < 7; j++)
	//		//{
	//		//	z[j]((double)j*j);
	//		//	cout <<"j:"<< j <<";op:"<< j*j <<" ctrl"<< (double&) z[j];
	//		//}
	//		//cout<< "----------------------------------"<< endl;
	//		//z.dump();
	//		//cout<< "----------------------------------"<< endl;
	//		//return 0;
	//		//multiarray<double> y(2, 7, 3);
	//		//for (int j = 0 ; j < 7; j++)
	//		//{
	//		//	for (int k = 0; k < 3; k++)
	//		//	{
	//		//		y[j][k] ((double)  j * k);
	//		//		cout <<"; j:"<< j << "; j:"<< k<< ";op:"<< j*k <<" ctrl"<< (double&) y[j][k];
	//		//		y[j][k] ((double&) y[j][k]);
	//		//	}
	//		//	cout<< endl<< "---------------------------------------"<< endl;
	//		//}
	//		//y.dump();
	//		//return 0;
	//		//multiarray<double> x(3, 4, 7, 3);
	//		//for (int i = 0; i < 4; i++)
	//		//{
	//		//	for (int j = 0 ; j < 7; j++)
	//		//	{
	//		//		for (int k = 0; k < 3; k++)
	//		//		{
	//		//			x[i][j][k] ((double) i * j * k);
	//		//			cout <<"i:"<< i << "; j:"<< j << "; j:"<< k<< ";op:"<< i*j*k <<" ctrl"<< (double&) x[i][j][k];
	//		//			//x[i][j][k] ((double) i * j * k);
	//		//		}
	//		//		cout<< endl<< "---------------------------------------"<< endl;
	//		//	}
	//		//	cout<< endl<< "*************************************************"<< endl;
	//		//}
	//		//x.dump();
	//	}
	//	{
	//		//MultiArray<double, 1> z(7);
	//		//for (int j = 0 ; j < 7; j++)
	//		//{
	//		//	z[j]((double)j*j);
	//		//	cout <<"j:"<< j <<";op:"<< j*j <<" ctrl"<< (double&) z[j];
	//		//}
	//		//cout<< endl;
	//		//cout<< "---------------- dump ------------------"<< endl;
	//		//z.dump();
	//		//cout<< "----------------------------------"<< endl;
	//		//return 0;
	//		//MultiArray<double, 2> y(7, 3);
	//		//for (int j = 0 ; j < 7; j++)
	//		//{
	//		//	for (int k = 0; k < 3; k++)
	//		//	{
	//		//		y[j][k] ((double)  j * k);
	//		//		cout <<"; j:"<< j << "; j:"<< k<< ";op:"<< j*k <<" ctrl"<< (double&) y[j][k];
	//		//		y[j][k] ((double&) y[j][k]);
	//		//	}
	//		//	cout<< endl<< "---------------------------------------"<< endl;
	//		//}
	//		//y.dump();
	//		//return 0;
	//		//MultiArray<double, 3> x(4, 7, 3);
	//		//		//multiarray<double> x(3, 4, 7, 3);
	//		//for (int i = 0; i < 4; i++)
	//		//{
	//		//	for (int j = 0 ; j < 7; j++)
	//		//	{
	//		//		for (int k = 0; k < 3; k++)
	//		//		{
	//		//			x[i][j][k] ((double) i * j * k);
	//		//			cout <<"i:"<< i << "; j:"<< j << "; j:"<< k<< ";op:"<< i*j*k <<" ctrl"<< (double&) x[i][j][k];
	//		//			//x[i][j][k] ((double) i * j * k);
	//		//		}//if (j ==1) break;
	//		//		cout<< "---------------------------------------"<< endl;
	//		//	}//if (i == 1)break;
	//		//	cout<< "*************************************************"<< endl;
	//		//}
	//		//cout<< ""<< endl;
	//		//cout<< "********************* dump *******************************"<< endl;
	//		//x.dump();
	//		//x.setdimensions(3, 8, 5);
	//		//		//multiarray<double> x(3, 4, 7, 3);
	//		//for (int i = 0; i < 3; i++)
	//		//{
	//		//	for (int j = 0 ; j < 8; j++)
	//		//	{
	//		//		for (int k = 0; k < 5; k++)
	//		//		{
	//		//			x[i][j][k] ((double) i+j+k+i * j * k);
	//		//			cout <<"i:"<< i << "; j:"<< j << "; j:"<< k<< ";op:"<< i*j*k <<" ctrl"<< (double&) x[i][j][k];
	//		//		}
	//		//		cout<< "---------------------------------------"<< endl;
	//		//	}
	//		//	cout<< "*************************************************"<< endl;
	//		//}
	//		//cout<< ""<< endl;
	//		//cout<< "********************* dump *******************************"<< endl;
	//		//x.dump();
	//	}
	//}catch(const char* xxx)
	//{
	//	cout<< "exception: "<< xxx<< endl;
	//}
	//return 0;