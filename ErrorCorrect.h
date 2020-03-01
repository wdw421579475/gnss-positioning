#pragma once
#include "TimeConvert.h"
#include "CordConvert.h"
#include "DecodeNovData.h"
#ifndef __ERRORCORRECT_H__
#define __ERRORCORRECT_H__
#define H0 0.0 //m
#define T0 15.0  //温度（摄氏）
#define K0 288.15 //温度（开氏温度）
#define p0 1013.25 //气压（mbar）
#define RH0 0.5//相对湿度


struct Obspospara
{
	//double H;//测站高度
	double T;//测站干温
	double p;//测站气压
	double RH;//测站相对湿度
};
double Klobutchar(IONO *ionodata, BLH *Obspos, GPSTime *t, double eleang, double aziang);
double Hopfield(double H, double eleang);


#endif