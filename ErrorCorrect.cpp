#include "stdafx.h"
#include "math.h"
#include "Calculate.h"
#include "DecodeNovData.h"
#include "CordConvert.h"
#include "Matrix.h"
#include "ErrorCorrect.h"

double Klobutchar(IONO *ionodata, BLH *Obspos, GPSTime *t, double eleang, double aziang)//时间、测站位置、高度角、方位角
{
	double eca;//地心角（单位半周）
	double latiI;//电离层穿透点纬度（单位半周）
	double longiI;
	double latiM;
	double tI;//电离层穿透点时间
	double AI;//电离层延迟振幅(单位s）
	double periodI;//电离层延迟周期
	double phaseI;//相位(单位rad）
	double F;//倾斜因子
	double IONDelay;//电离层延迟
					//Calculate the earth-centred angle 
	eca = (0.0137 / (eleang / pi + 0.11)) - 0.022;
	//Compute the latitude of Ionospheric Pierce Point(IPP)
	latiI = Obspos->latitude / pi + eca*cos(aziang);
	if (latiI > 0.416)
	{
		latiI = 0.416;
	}
	if (latiI < -0.416)
	{
		latiI = -0.416;
	}
	//Compute the longitude of the IPP
	longiI = Obspos->longitude / pi + eca*sin(aziang) / cos(latiI);
	//Find the geomagnetic latitude of the IPP
	latiM = latiI + 0.064*cos((longiI - 1.617)*pi);
	//Find the local time at the IPP
	if (t->SecOfWeek >= 86400)
	{
		t->SecOfWeek = t->SecOfWeek - 86400;
	}
	tI = 43200 * longiI + t->SecOfWeek;
	if (tI > 86400)
	{
		tI = tI - 86400;
	}
	if (tI < 0)
	{
		tI = tI + 86400;
	}
	//Compute the amplitude of ionospheric delay
	AI = ionodata->a[0] * 1 + ionodata->a[1] * latiM + ionodata->a[2] * latiM*latiM + ionodata->a[3] * latiM*latiM*latiM;
	if (AI < 0)
	{
		AI = 0;
	}
	//Compute the period of ionospheric delay
	periodI = ionodata->b[0] * 1 + ionodata->b[1] * latiM + ionodata->b[2] * latiM*latiM + ionodata->b[3] * latiM*latiM*latiM;
	if (periodI < 72000)
	{
		periodI = 72000;
	}
	//Compute the phase of ionospheric delay
	phaseI = 2 * pi*(tI - 50400) / periodI;
	//Compute the slant factor
	F = 1.0 + 16.0*(0.53 - eleang)*(0.53 - eleang)*(0.53 - eleang);
	//Compute the ionospheric time delay
	if (phaseI <= 1.57)
	{
		IONDelay = (5e-9 + AI*(1 - phaseI*phaseI / 2 + phaseI*phaseI*phaseI*phaseI / 24))*F;
	}
	if (phaseI >= 1.57)
	{
		IONDelay = 5e-9*F;
	}
	else
	{
		IONDelay = 0;
	}
	return IONDelay;
}


double Hopfield(double H, double eleang)//测站高度、高度角
{
	double hd, Kd, hw, Kw, ee;
	double delta_trop;
	double T;//测站干温
	double p;//测站气压
	double RH;//测站相对湿度

	if (H < -1000)
	{
		H = -1000;
	}
	RH = RH0*exp(-0.0006396*(H - H0));

	if (0.0000226*(H - H0) > 1)
	{
		p = 0;//若高度超出对流层，气压置零
	}
	else
	{
		p = p0*pow((1 - 0.0000226*(H - H0)), 5.225);
	}

	T = T0 - 0.0065*(H - H0) + 273.16;
	ee = RH*exp(-37.2465 + 0.213166*T - 0.000256908*T*T);
	hw = 11000.0;
	hd = 40316 + 148.72*(T0 - 273.16);
	Kd = 155.2e-7*p*(hd - H) / T;
	Kw = 155.2e-7*4810.0*ee*(hw - H) / (T*T);
	delta_trop = Kd / (sin(sqrt((eleang*eleang + 6.25) / 180 * pi))) + Kw / (sin(sqrt((eleang*eleang + 2.25) / 180 * pi)));
	return delta_trop;
}