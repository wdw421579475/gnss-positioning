#pragma once
#include "TimeConvert.h"
#include "CordConvert.h"
#include "DecodeNovData.h"
#ifndef __CALCULATE_H__
#define __CALCULATE_H__
#define pi 3.1415926535898
#define c 2.99792458e+8     //光速
#define miu 3.986004415e+14   //GM(m^3/s^2)
#define omegaE 7.2921151467e-5  //地球自转角速度(rad/s)

struct Velocity
{
	double vx;
	double vy;
	double vz;
	double deltaTsv;//钟差
	double clkvel;//钟速
};




int SatPosVelCal(GPSEphem Eph[], XYZ SatPos[], Velocity Vsat[], Obsdata *obs, int i);






#endif
