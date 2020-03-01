#pragma once
#ifndef __DECODENOVDATA_H__
#define __DECODENOVDATA_H__
#include "TimeConvert.h"
#include "Serial.h"
#define MAXCHANNEL 32
struct ObsPRN
{
	unsigned short PRN;//卫星PRN号
	double psr;//伪距
	float psr_std;//伪距标准差
	double adr;//载波相位
	float adr_std;//载波相位标准差
	float DopplerShift;//多普勒频移
	float CNR;//载噪比

};
struct Obsdata  //观测数据
{
	GPSTime ObsTime;

	long obsnum;
	ObsPRN obsPRN[MAXCHANNEL];
	//double ChannelState;//通道状态

};

struct GPSEphem //卫星星历
{
	short PRN;//卫星PRN号
	unsigned int IODE1, IODE2, IODC;
	GPSTime TOE, TOC, T, TOW;
	double A, deltaN, M0, ecc, omega;
	double cuc, cus, crc, crs, cic, cis;
	double I0, I_o, omega0, omega_o, tgd;//I_o为轨道倾角速度
	double af0, af1, af2, N, URA;
};

struct PositionRESULT //定位结果
{
	GPSTime PosTime;//定位时间

	double lat, lon, hgt;

	double sigma, pdop;
	float lat_std, lon_std, hgt_std, sol_age, diff_age;
	short solnSVs, TrackSVs;
};

struct IONO//电离层参数
{
	double a[4];//alpha_parameter
	double b[4];//belta_parameter
	bool Valid;
	IONO()
	{
		Valid = false;
	}
};
int DecodeNovData(unsigned char buff[], GPSEphem Eph[], Obsdata *Obs, PositionRESULT *PosRes, IONO *Ion, int n, int *s);
double R8(unsigned char *p);
short I2(unsigned char *p);
unsigned short U2(unsigned char *p);
unsigned int U4(unsigned char *p);
float F4(unsigned char *p);
unsigned int crc32(const unsigned char *buff, int len);

#endif // !__READFILE_H__

