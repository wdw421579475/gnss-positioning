//***********Ê±¼ä×ª»»***********//
#include "stdafx.h"
#include "math.h"
#include "TimeConvert.h"

void CommonTimetoMJDTime(CommonTime *t1, MJDTime *t2)
{
	double JD, MJD;
	unsigned short year, month;
	if (t1->Month <= 2)
	{
		year = t1->Year - 1;
		month = t1->Month + 12;
	}
	if (t1->Month>2)
	{
		year = t1->Year;
		month = t1->Month;
	}
	JD = int(365.25*year) + int(30.6001*(month + 1)) + t1->Day + t1->Hour / 24.0 +
		t1->Minute / (24.0 * 60.0) + t1->Second / (24.0 * 60.0*60.0) + 1720981.5;
	MJD = JD - 2400000.5;
	t2->Days = int(MJD);
	t2->FracDay = t1->Hour / 24.0 + t1->Minute / (24.0 * 60.0) + t1->Second / (24.0 * 60.0*60.0);
	return;


}
void MJDTimetoCommonTime(MJDTime *t1, CommonTime *t2)
{
	int A, B, C, D, E;
	double JD, UT;
	JD = t1->Days + t1->FracDay + 2400000.5;
	A = int(JD + 0.5);
	B = A + 1537;
	C = int((B - 122.1) / 365.25);
	D = int(365.25*C);
	E = int((B - D) / 30.6001);
	UT = 24 * t1->FracDay;
	t2->Day = B - D - int(30.6001*E);
	t2->Month = E - 1 - 12 * int(E / 14);
	t2->Year = C - 4715 - int((7 + t2->Month) / 10);
	t2->Hour = int(UT);
	t2->Minute = int(60 * (UT - t2->Hour));
	t2->Second = 60 * (60 * (UT - t2->Hour) - t2->Minute);

}
void MJDTimetoGPSTime(MJDTime *t1, GPSTime *t2)
{
	double mjd;
	mjd = t1->Days + t1->FracDay;
	t2->Week = int((mjd - 44244) / 7);
	t2->SecOfWeek = (mjd - 44244 - (t2->Week) * 7) * 86400;
}
void GPSTimetoMJDTime(GPSTime *t1, MJDTime *t2)
{
	double mjd;
	mjd = 44244 + (t1->Week) * 7 + (t1->SecOfWeek) / 86400;
	t2->Days = int(mjd);
	t2->FracDay = mjd - t2->Days;
}