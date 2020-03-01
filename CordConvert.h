#pragma once
#ifndef __CORDCONVERT_H__
#define __CORDCONVERT_H__
#define PI 3.141592653589
#define R_WGS84  6378137.0          /* Radius Earth [m]; WGS-84  */
#define F_WGS84  1.0/298.257223563  /* Flattening; WGS-84   */
#define Omega_WGS 7.2921151467e-5   /*[rad/s], the earth rotation rate */
#define GM_Earth   398600.5e+9     /* [m^3/s^2]; WGS-84 */

struct XYZ
{
	double x;
	double y;
	double z;
};
struct BLH
{
	double longitude;//经度（单位弧度）
	double latitude;//纬度（单位弧度）
	double h;//高度（单位米）
};

double JWtoRAD(double d, double f, double m);
void BLHtoXYZ(BLH *c1, XYZ *c2);
void XYZtoBLH(XYZ *c1, BLH *c2);




#endif // !__CORDCONVERT_H__

