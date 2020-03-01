#pragma once
#include "TimeConvert.h"
#include "CordConvert.h"
#include "DecodeNovData.h"
#include "Calculate.h"
#include "stdafx.h"
#include "Matrix.h"
#include "RTCMDecode.h"
#ifndef __SPP_H__
#define __SPP_H__

struct sppresult
{
	XYZ xyzspppos;
	BLH blhspppos;//接收机定位位置结果
	double dt;//接收机钟差（单位s）
	double dp;//接收机钟差（单位m）
	double P_Dop;
	double sigma;
	double P_Pdop;//精度因子
};
struct velresult
{
	double vx;
	double vy;
	double vz;
	double dp_o;
	double dt_o;
	double V_Dop;
	double V_Pdop;
	double sigma;

};
int sppposition(GPSEphem Eph[], Obsdata *obs, Velocity Vsat[], XYZ position[], PositionRESULT *posres, sppresult *res, IONO *ion);
int dspp(GPSEphem Eph[], Obsdata *obs, Velocity Vsat[], XYZ position[], PositionRESULT *posres, sppresult *res, RTCMRES *rtcmres);
int sppvelocity(GPSEphem Eph[], Velocity Vsat[], Obsdata *obs, XYZ position[], sppresult *res, velresult *vres);
#endif
