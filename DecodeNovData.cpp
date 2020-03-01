#include "stdafx.h"
#include "math.h"
#include "DecodeNovData.h"
#define MAXLEN 1024
#define POLYCRC32   0xEDB88320u /* CRC32 polynomial */

//********************注释***********************//
/*
fp 二进制数据文件指针
Eph[]  星历数据  32
Obs  观测数据
PosResult 定位结果
ION 电离层数据

返回值
0：数据读完，有问题
1：crc校验不通过
2：观测数据
3：电离层
4：定位结果
5：星历数据
*/
int DecodeNovData(unsigned char buff[], GPSEphem Eph[], Obsdata *Obs, PositionRESULT *PosRes, IONO *Ion, int n, int *s)
{
	int ID, RetVal;
	int p = *s;
	int i = 0;
	int j;
	short k;
	int lenH;//数据头长度
	int lenD;//message length
	int LEN;//数据总长度
	unsigned char buffer[MAXLEN];
	float crc;
	
	
	
		//读数据找同步符（循环）
		do
		{
			/*if (fread(buff + 2, sizeof(unsigned char), 1, fp) != 1)
			{
			return 0;
			}*/
			if (buff[p] == 0xAA && buff[p + 1] == 0x44 && buff[p + 2] == 0x12)
			{
				break;
			}
			else 
			{
				p++;
			}
		} while (p <= n - 3);

	if (n - p <= 28)  //本次数据读取完
	{
		p = 0;
		*s = 0;
		return 1;
	}
	for (k = 0 ;k < 28; k++)
	{
		buffer[k] = buff[p];
		p++;
	}
	lenH = 28;
	lenD = U2(buffer + 8);
	if (n - p < lenD + 4)
	{
		p = 0;
		*s = p;
		return 1;

	}
	for (k = 28; k < lenD + 32; k++)
	{
		buffer[k] = buff[p];
		p++;
	}
	*s = p;
	//一组数据长度
	LEN = 4 + lenD + lenH;

	//CRC校验，不通过返回0
	crc = U4(buffer + lenD + 28);
	if (crc32(buffer , lenD + 28) != crc)
	{
		return 0;
	}
	

	//find ID
	ID = U2(buffer + 4);
	//分支，不同数据解码，返回解码结果
	switch (ID)
	{
	case 7://GPSEPHEM
		RetVal = 5;
		
		break;
	case 8://IONUTC
		RetVal = 3;
		break;
	case 16://CLOCKMODEL

		break;
	case 25://RAWGPSSUBFRAME

		break;
	case 26://CLOCKSTEERING

		break;
	case 37://VERSION

		break;
	case 41://RAWEPHEM

		break;
	case 42://BESTPOS

		break;
	case 43://RANGE
		RetVal = 2;
		break;
	case 47://PSRPOS
		RetVal = 4;
		break;
	case 140://RANGECMP
		RetVal = 2;
		break;

	}
	//return RetVal;
	//根据不同的数据类型，运行子函数，提取不同类型数据
	if (RetVal == 5)//数据为星历数据
	{
		i = U4(buffer + 28) - 1;
		Eph[i].PRN = U4(buffer + 28);
		Eph[i].IODE1 = U4(buffer + 16 + 28);
		Eph[i].IODE2 = U4(buffer + 28 + 20);
		Eph[i].TOC.Week = U4(buffer + 28 + 24);
		Eph[i].TOC.SecOfWeek = R8(buffer + 28 + 164);
		Eph[i].TOE.Week = U4(buffer + 28 + 24);
		Eph[i].TOE.SecOfWeek = R8(buffer + 28 + 32);
		Eph[i].TOW.Week = U4(buffer + 28 + 24);
		Eph[i].TOW.SecOfWeek = R8(buffer + 28 + 4);
		Eph[i].A = R8(buffer + 28 + 40);
		Eph[i].deltaN = R8(buffer + 28 + 48);
		Eph[i].M0 = R8(buffer + 28 + 56);
		Eph[i].ecc = R8(buffer + 28 + 64);
		Eph[i].omega = R8(buffer + 28 + 72);
		Eph[i].cuc = R8(buffer + 28 + 80);
		Eph[i].cus = R8(buffer + 28 + 88);
		Eph[i].crc = R8(buffer + 28 + 96);
		Eph[i].crs = R8(buffer + 28 + 104);
		Eph[i].cic = R8(buffer + 28 + 112);
		Eph[i].cis = R8(buffer + 28 + 120);
		Eph[i].I0 = R8(buffer + 28 + 128);
		Eph[i].I_o = R8(buffer + 28 + 136);
		Eph[i].omega0 = R8(buffer + 28 + 144);
		Eph[i].omega_o = R8(buffer + 28 + 152);
		Eph[i].IODC = U4(buffer + 28 + 160);
		Eph[i].af0 = R8(buffer + 28 + 180);
		Eph[i].af1 = R8(buffer + 28 + 188);
		Eph[i].af2 = R8(buffer + 28 + 196);
		Eph[i].N = R8(buffer + 28 + 208);
		Eph[i].URA = R8(buffer + 28 + 216);
		Eph[i].tgd = R8(buffer + 28 + 172);
		Eph[i].T.Week = U2(buffer + 14);
		Eph[i].T.SecOfWeek = U4(buffer + 16) / 1000;
		
		return 5;

	}
	if (RetVal == 2)//数据为观测数据
	{
		/*Obs->obsnum = F4(buffer + 28);
		Obs->ObsTime.Week = U2(buffer + 14);
		Obs->ObsTime.SecOfWeek = U4(buffer + 16);

		for (i = 0;i <= int(Len/44);i++)
		{
		Obs->obsPRN[i].PRN = U2(buff + 28 + 4 + 44 * i);
		Obs->obsPRN[i].psr = R8(buff + 28 + 8 + 44 * i);
		Obs->obsPRN[i].psr_std = F4(buff + 28 + 16 + 44 * i);
		Obs->obsPRN[i].adr = R8(buff + 28 + 20 + 44 * i);
		Obs->obsPRN[i].adr_std = F4(buff + 28 + 28 + 44 * i);
		Obs->obsPRN[i].DopplerShift = F4(buff + 28 + 32 + 44 * i);
		Obs->obsPRN[i].CNR = F4(buff + 28 + 36 + 44 * i);

		}*/
		Obs->obsnum = F4(buffer + 28);
		Obs->ObsTime.Week = U2(buffer + 14);
		Obs->ObsTime.SecOfWeek = U4(buffer + 16) / 1000;
		for (i = 0; i <= int(LEN / 44); i++)
		{
			j = U2(buffer + 28 + 4 + 44 * i) - 1;
			if (j < 32)
			{
				Obs->obsPRN[j].PRN = U2(buffer + 28 + 4 + 44 * i);
				Obs->obsPRN[j].psr = R8(buffer + 28 + 8 + 44 * i);
				Obs->obsPRN[j].psr_std = F4(buffer + 28 + 16 + 44 * i);
				Obs->obsPRN[j].adr = R8(buffer + 28 + 20 + 44 * i);
				Obs->obsPRN[j].adr_std = F4(buffer + 28 + 28 + 44 * i);
				Obs->obsPRN[j].DopplerShift = F4(buffer + 28 + 32 + 44 * i);
				Obs->obsPRN[j].CNR = F4(buffer + 28 + 36 + 44 * i);
			}

		}
		
		return 2;
	}
	if (RetVal == 4)//数据为定位结果
	{
		PosRes[i].hgt = R8(buffer + 28 + 24);
		PosRes[i].lon = R8(buffer + 28 + 16);
		PosRes[i].lat = R8(buffer + 28 + 8);
		PosRes[i].lat_std = F4(buffer + 28 + 40);
		PosRes[i].lon_std = F4(buffer + 28 + 44);
		PosRes[i].hgt_std = F4(buffer + 28 + 48);
		PosRes[i].diff_age = F4(buffer + 28 + 56);
		//PosRes[i].solnSVs = unsigned char(buffer + 28 + 65);
		PosRes[i].sol_age = F4(buffer + 28 + 60);
		//PosRes[i].sigma = unsigned char(buffer + 28 + 71);
		
		return 4;
	}
	if (RetVal == 4)//数据为电离层参数
	{
		Ion->a[0] = R8(buffer + 28);
		Ion->a[1] = R8(buffer + 28 + 8);
		Ion->a[2] = R8(buffer + 28 + 16);
		Ion->a[3] = R8(buffer + 28 + 24);
		Ion->b[0] = R8(buffer + 28 + 32);
		Ion->b[1] = R8(buffer + 28 + 40);
		Ion->b[2] = R8(buffer + 28 + 48);
		Ion->b[3] = R8(buffer + 28 + 56);
		return 5;
		
	}

}
//**************定义数据结构函数*****************//
double R8(unsigned char *p)
{
	double r;
	memcpy(&r, p, 8);
	return r;
}
short I2(unsigned char *p)
{
	short r;
	memcpy(&r, p, 2);
	return r;
}
unsigned short U2(unsigned char *p)
{
	short r;
	memcpy(&r, p, 2);
	return r;
}
unsigned int U4(unsigned char *p)
{
	int r;
	memcpy(&r, p, 4);
	return r;
}
float F4(unsigned char *p)
{
	float r;
	memcpy(&r, p, 4);
	return r;
}
//************CRC校验函数**************//
unsigned int crc32(const unsigned char *buff, int len)
{
	int i, j;
	unsigned int crc = 0;
	for (i = 0; i<len; i++)
	{
		crc ^= buff[i];
		for (j = 0; j<8; j++) {
			if (crc & 1) crc = (crc >> 1) ^ POLYCRC32;
			else crc >>= 1;
		}
	}
	return crc;
}
