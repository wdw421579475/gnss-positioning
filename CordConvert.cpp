#include "stdafx.h"
#include "math.h"
#include "CordConvert.h"

#define pi 3.1415926535898
double JWtoRAD(double d, double f, double m)
{
	double r;
	r = PI*(d + f / 60.0 + m / 3600.0) / 180.0;
	return r;
}
void BLHtoXYZ(BLH *c1, XYZ *c2)
{
	double a, b, e, c, W, N, B, L, H;
	a = 6378137.0;
	b = 6356752.3142;
	e = sqrt(a*a - b*b) / a;
	c = a*a / b;
	L = c1->longitude;
	B = c1->latitude;
	H = c1->h;
	W = sqrt(1 - e*e*sin(B)*sin(B));
	N = a / W;
	c2->x = (N + c1->h)*cos(B)*cos(L);
	c2->y = (N + c1->h)*cos(B)*sin(L);
	c2->z = (N*(1.0 - e*e) + H)*sin(B);
}

void XYZtoBLH(XYZ *c1, BLH *c2)
{
	if (c1->x == NULL || c1->y == NULL || c1->z == NULL)
	{
		c2->latitude = 0;
		c2->longitude = 0;
		c2->h = 0;
	}
	int j = 0;
	double a, b, e, c, W, N, fai, R, h0, B0, B;
	a = 6378137.0;
	b = 6356752.3142;
	e = sqrt(a*a - b*b) / a;
	c = a*a / b;
	R = sqrt(c1->x*c1->x + c1->y*c1->y + c1->z*c1->z);
	fai = atan(c1->z / sqrt(c1->x*c1->x + c1->y*c1->y));
	h0 = 0.0;
	B0 = atan2(c1->z, R);
	while (j<1000)   //迭代计算纬度B
	{
		W = sqrt(1 - e*e*sin(B0)*sin(B0));
		B = atan(tan(fai)*(1 + a*e*e*sin(B0) / (c1->z*W)));
		if (B - B0 < 1.0e-10)
			break;
		B0 = B;
		j++;
	}
	c2->latitude = B;
	N = a / W;
	c2->longitude = atan2(c1->y, c1->x);//计算经度L
	c2->h = R*cos(fai) / cos(B) - N;//计算h


}

/*void XYZtoBLH(XYZ *c1, BLH *c2)
{

double  b0, n, w;
double fai;
double abs;
double a1 = 6378137.0;
double ee = 0.00669437999013;



fai = atan2(c1->z, sqrt(c1->x*c1->x + c1->y*c1->y));

if (c1->x != 0)
{
c2->longitude = atan2(c1->y, c1->x);
}
else
{
c2->longitude = 0;
}


b0 = atan2(c1->z, a1);
do
{
w = sqrt((1 - ee*sin(b0)*sin(b0)));
n = a1 / w;
if (c1->z == 0)
{
c2->latitude = 0;
}
else
{
c2->latitude = atan2(tan(fai)*(1 + a1 * ee*sin(b0)), (c1->z*w));
}
abs = c2->latitude - b0;
b0 = c2->latitude;
} while (fabs(abs)>pow(10, -10));

while (c2->longitude < -pi)
{
c2->longitude = c2->longitude + 2 * pi;
}
while (c2->longitude > pi)
{
c2->longitude = c2->longitude - 2 * pi;
}

while (c2->latitude < -pi / 2)
{
c2->latitude = c2->latitude + pi;
}
while (c2->latitude > pi / 2)
{
c2->latitude = c2->latitude - pi;
}

c2->h = sqrt(c1->x*c1->x + c1->y*c1->y + c1->z*c1->z)*cos(fai) / cos(c2->latitude) - n;



}
*/