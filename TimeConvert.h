#pragma once
#ifndef __TIMECONVERT_H__
#define __TIMECONVERT_H__
struct CommonTime
{
	unsigned short Year;
	unsigned short Month;
	unsigned short Day;
	unsigned short Hour;
	unsigned short Minute;
	double Second;
};
struct MJDTime
{
	int Days;
	double FracDay;
	MJDTime()
	{
		Days = 0;
		FracDay = 0.0;
	}

};
struct GPSTime
{
	unsigned short Week;
	double SecOfWeek;
	GPSTime()
	{
		Week = 0;
		SecOfWeek = 0.0;
	}
};

void MJDTimetoCommonTime(MJDTime *t1, CommonTime *t2);
void CommonTimetoMJDTime(CommonTime *t1, MJDTime *t2);
void MJDTimetoGPSTime(MJDTime *t1, GPSTime *t2);
void GPSTimetoMJDTime(GPSTime *t1, MJDTime *t2);

#endif // !__TIMECONVERT_H__

