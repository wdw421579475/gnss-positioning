// SppintoIntegrado.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h> 
#include <Windows.h>
#include <time.h>
#include "Matrix.h"
#include "Time.h"
#include "CordConvert.h"
#include "DecodeNovData.h"
#include "Calculate.h"
#include "ErrorCorrect.h"
#include "spp.h"
#include "RTCMDecode.h"
#include "sockets.h"

#define buff_max 8192//设置缓冲区最大容量
#define MAXLENOFDATA 8192
/*
int main()
{
	//FILE* fp = 0;
	FILE* Fout = 0;

	CSerial Serial;
	CfgInfo cfgInfo;

	GPSEphem Eph[32];
	XYZ Satpos[32];
	Velocity Satvel[32];
	Obsdata obsdat;
	sppresult result;
	velresult vresult;
	PositionRESULT positionres;
	IONO ionres;
	
	Velocity SatData;
	int i, j = 0;
	int k = 0;
	int n;
	int sn;//卫星数
	int retval;
	
	double tgps_r[32] = { 0 };
	double dB, dL, dH;//计算解算结果和参考结果差值
	double t;//计算每次sleep时间
	XYZ xyz;
	BLH blh;

	clock_t start, end;

	memset(Eph, 0, sizeof(GPSEphem));
	memset(Satpos, 0, sizeof(XYZ));
	memset(Satvel, 0, sizeof(Velocity));
	memset(Satpos, 0, sizeof(Satpos));
	memset(Satvel, 0, sizeof(Satvel));
	memset(&obsdat, 0, sizeof(obsdat));
	memset(&ionres, 0, sizeof(ionres));

	unsigned char buff[buff_max];
	char str[50];
	int len;
	int *s;
	s = &k;

	if (Serial.Open(cfgInfo.gpsPort, cfgInfo.gpsBaud) == false)
	{
		printf("The GPS COM%d was not opened.\n", cfgInfo.gpsPort);
		return 0;
	}
    //发送星历参数
	strcpy_s(str, "LOG GPSEPHEMB ONCHANGED");
	len = strlen(str);
	str[len++] = 0x0d;
	str[len++] = 0x0a;
	Serial.SendData(str, len);
	memset(str, 0, 50);
	
	//发送电离层参数
	strcpy_s(str, "LOG IONUTCB ONCHANGED");
	len = strlen(str);
	str[len++] = 0x0d;
	str[len++] = 0x0a;
	Serial.SendData(str, len);
	memset(str, 0, 50);
	Sleep(4200);//读取星历
	
	//发送观测数据
	strcpy_s(str, "LOG RANGEB ONTIME 1");
	len = strlen(str);
	str[len++] = 0x0d;
	str[len++] = 0x0a;
	Serial.SendData(str, len);
	memset(str, 0, 50);

	//发送定位结果数据
	strcpy_s(str, "LOG PSRPOSB ONTIME 1");
	len = strlen(str);
	str[len++] = 0x0d;
	str[len++] = 0x0a;
	Serial.SendData(str, len);
	memset(str, 0, 50);

	while (1)
	{
		start = clock();
		n = Serial.ReadData(buff, 8192);//缓冲区读取到的数据长度
		
		retval = 0;
		while (retval != 1)//数据未读完，进行下一组数据读入
		{
			retval = DecodeNovData(buff, Eph, &obsdat, &positionres, &ionres, n, s);
			
			if (retval == 2)//得到观测数据
			{
				sn = 0;
				for (j = 0; j < 32; j++)
				{
					if (obsdat.obsPRN[j].PRN > 0 && obsdat.obsPRN[j].PRN < 33 && Eph[j].PRN == obsdat.obsPRN[j].PRN && obsdat.obsPRN[j].psr < 1e10)//加入判定防止伪距过大错误
					{
						sn++;
					}
				}
					//参与解算的卫星数少于4颗，解算失败
				if (sn < 4)
				{
					continue;
					//printf("卫星数量不足，解算失败\n");
				}
					//参与解算的卫星数大于4颗，最小二乘解算
				if (sn >= 4)
				{

						for (i = 0;i < 32;i++)
						{
							SatPosVelCal(Eph, Satpos, Satvel, &obsdat, i);
						}
						sppposition(Eph, &obsdat, Satvel, Satpos, &positionres, &result, &ionres);
						sppvelocity(Eph, Satvel, &obsdat, Satpos, &result, &vresult);

						xyz.x = result.xyzspppos.x;
						xyz.y = result.xyzspppos.y;
						xyz.z = result.xyzspppos.z;
						XYZtoBLH(&xyz, &blh);
						dB = blh.latitude*180/pi - positionres.lat;
						dL = blh.longitude*180/pi - positionres.lon;
						dH = blh.h - positionres.hgt;

						printf("t = %f ,dB = %lf,  dL = %lf,  dH = %lf\n", obsdat.ObsTime.SecOfWeek, dB, dL, dH);
						//fprintf(Fout, "%d %f %f %f\n", j, blh.latitude, blh.longitude, blh.h);
						//j++;
						memset(Satpos, 0, sizeof(Satpos));
						memset(Satvel, 0, sizeof(Satvel));
						memset(&obsdat, 0, sizeof(obsdat));
						memset(&ionres, 0, sizeof(ionres));//眼花缭乱
				}
			}
			
		}
		end = clock();
		t = 1000 - (end - start);
		memset(buff, 0, 8192);
		Sleep(t);
	}

	//*if ((Fout = fopen("result.txt", "wt")) == NULL)
	//{
	//	printf("Cannot open output file.\n");
	//	return 0;
	//}
	//fclose(fp);

	fclose(Fout);

	return 0;

};
*/
int main()
{
	int socknum = 0;
	char buff[MAXLENOFDATA];
	unsigned char buff2[MAXLENOFDATA];
	RTCMRES rtcmres;
	SOCKET sock;
	
	FILE* Fout1 = 0;
	FILE* Fout2 = 0;
	FILE* Fout3 = 0;//输出结果
	CSerial Serial;
	CfgInfo cfgInfo;

	GPSEphem Eph[32];
	XYZ Satpos[32];
	Velocity Satvel[32];
	Obsdata obsdat;
	sppresult result;//spp定位结果
	sppresult result2;//差分改正后spp定位结果
	velresult vresult;
	PositionRESULT positionres;
	IONO ionres;

	Velocity SatData;
	int i, j = 0;
	int k = 0;
	int n;
	int sn;//卫星数
	int retval;

	double tgps_r[32] = { 0 };
	//double dB, dL, dH;//计算解算结果和参考结果差值
	double t;//计算每次sleep时间
	XYZ xyz, xyz2;
	BLH blh, blh2;

	clock_t start, end;
	unsigned char buff0[buff_max];
	char str[50];
	int len;
	int *s;
	s = &k;

	memset(Eph, 0, sizeof(GPSEphem));
	memset(Satpos, 0, sizeof(XYZ));
	memset(Satvel, 0, sizeof(Velocity));
	memset(Satpos, 0, sizeof(Satpos));
	memset(Satvel, 0, sizeof(Satvel));
	memset(&obsdat, 0, sizeof(obsdat));
	memset(&ionres, 0, sizeof(ionres));
	memset(&rtcmres, 0, sizeof(rtcmres));
	if (Serial.Open(cfgInfo.gpsPort, cfgInfo.gpsBaud) == false)
	{
		printf("The GPS COM%d was not opened.\n", cfgInfo.gpsPort);
		return 0;
	}
	if (OpenDGPSSocket(sock) == false)
	{
		printf("无法建立网络链路\n");
		return 0;
	}
	//发送星历参数
	strcpy_s(str, "LOG GPSEPHEMB ONCHANGED");
	len = strlen(str);
	str[len++] = 0x0d;
	str[len++] = 0x0a;
	Serial.SendData(str, len);
	memset(str, 0, 50);


	//发送电离层参数
	strcpy_s(str, "LOG IONUTCB ONCHANGED");
	len = strlen(str);
	str[len++] = 0x0d;
	str[len++] = 0x0a;
	Serial.SendData(str, len);
	memset(str, 0, 50);
	Sleep(4200);//读取星历


	//发送观测数据
	strcpy_s(str, "LOG RANGEB ONTIME 1");
	len = strlen(str);
	str[len++] = 0x0d;
	str[len++] = 0x0a;
	Serial.SendData(str, len);
	memset(str, 0, 50);

	//发送定位结果数据
	strcpy_s(str, "LOG PSRPOSB ONTIME 1");
	len = strlen(str);
	str[len++] = 0x0d;
	str[len++] = 0x0a;
	Serial.SendData(str, len);
	memset(str, 0, 50);

	Fout1 = fopen("C:\\Users\\Administrator\\Desktop\\professional Files\\卫星导航课程设计2\\数据结果\\sppresult.txt", "w+");
	Fout2 = fopen("C:\\Users\\Administrator\\Desktop\\professional Files\\卫星导航课程设计2\\数据结果\\dgpssppresult.txt", "w+");
	Fout3 = fopen("C:\\Users\\Administrator\\Desktop\\professional Files\\卫星导航课程设计2\\数据结果\\referenceresult.txt", "w+");
	while (1)
	{
		
		//读取差分改正数据
		socknum = recv(sock, buff , MAXLENOFDATA , 0);
		//Sleep(1000);
		for (i = 0;i < MAXLENOFDATA;i++)
		{
			buff2[i] = buff[i];
		}
		memset(buff, 0, sizeof(char) * MAXLENOFDATA);
		DecodeRTCMData(buff2, &rtcmres, MAXLENOFDATA);
		//printf("%lf,%d,%d\n", rtcmres.head.modZ, rtcmres.head.LEN, rtcmres.head.SN);
		
		start = clock();
		n = Serial.ReadData(buff0, 8192);//缓冲区读取到的数据长度
		printf("%d\n", n);
		retval = 0;
		while (retval != 1)//数据未读完，进行下一组数据读入
		{
			retval = DecodeNovData(buff0, Eph, &obsdat, &positionres, &ionres, n, s);

			if (retval == 2)//得到观测数据
			{
				sn = 0;
				for (j = 0; j < 32; j++)
				{
					if (obsdat.obsPRN[j].PRN > 0 && obsdat.obsPRN[j].PRN < 33 && Eph[j].PRN == obsdat.obsPRN[j].PRN && obsdat.obsPRN[j].psr < 1e10)//加入判定防止伪距过大错误
					{
						sn++;
					}
				}
				//参与解算的卫星数少于4颗，解算失败
				if (sn < 4)
				{
					continue;
					//printf("卫星数量不足，解算失败\n");
				}
				//参与解算的卫星数大于4颗，最小二乘解算
				if (sn >= 4)
				{

					for (i = 0;i < 32;i++)
					{
						SatPosVelCal(Eph, Satpos, Satvel, &obsdat, i);
					}
					sppposition(Eph, &obsdat, Satvel, Satpos, &positionres, &result, &ionres);
					dspp(Eph, &obsdat, Satvel, Satpos, &positionres, &result2, &rtcmres);
					//sppvelocity(Eph, Satvel, &obsdat, Satpos, &result, &vresult);
					/*xyz.x = result.xyzspppos.x;
					xyz.y = result.xyzspppos.y;
					xyz.z = result.xyzspppos.z;

					xyz2.x = result2.xyzspppos.x;
					xyz2.y = result2.xyzspppos.y;
					xyz2.z = result2.xyzspppos.z;

					XYZtoBLH(&xyz, &blh);
					XYZtoBLH(&xyz2, &blh2);*/
					/*
					dB = blh.latitude * 180 / pi - positionres.lat;
					dL = blh.longitude * 180 / pi - positionres.lon;
					dH = blh.h - positionres.hgt;
					*/
					blh.latitude = result.blhspppos.latitude * 180 / pi;
					blh.longitude = result.blhspppos.longitude * 180 / pi;
					blh.h = result.blhspppos.h;

					blh2.latitude = result2.blhspppos.latitude * 180 / pi;
					blh2.longitude = result2.blhspppos.longitude * 180 / pi;
					blh2.h = result2.blhspppos.h;

					printf("t = %f\n", obsdat.ObsTime.SecOfWeek);
					printf("接收机定位参考结果：\nB = %lf L = %lf H = %lf\n", positionres.lat, positionres.lon, positionres.hgt);
					printf("spp定位结果:\nB = %lf L = %lf H = %lf PDOP = %lf\n", blh.latitude, blh.longitude , blh.h, result.P_Pdop);
					printf("差分改正spp定位结果:\nB = %lf L = %lf H = %lf PDOP = %lf\n", blh2.latitude, blh2.longitude, blh2.h, result2.P_Pdop);
					//fprintf(Fout, "%d %f %f %f\n", j, blh.latitude, blh.longitude, blh.h);
					//j++;
					fprintf(Fout1, "%f %f %f %f %.8f %.8f %.8f %f\n", obsdat.ObsTime.SecOfWeek, result.xyzspppos.x, result.xyzspppos.y, result.xyzspppos.z, blh.latitude, blh.longitude, blh.h, result.P_Pdop);
					fprintf(Fout2, "%f %f %f %f %.8f %.8f %.8f %f\n", obsdat.ObsTime.SecOfWeek, result2.xyzspppos.x, result2.xyzspppos.y, result2.xyzspppos.z, blh2.latitude, blh2.longitude, blh2.h, result2.P_Pdop);
					fprintf(Fout3, "%f %f %f %f\n", obsdat.ObsTime.SecOfWeek, positionres.lat, positionres.lon, positionres.hgt);
				
					memset(Satpos, 0, sizeof(Satpos));
					memset(Satvel, 0, sizeof(Satvel));
					memset(&obsdat, 0, sizeof(obsdat));
					memset(&ionres, 0, sizeof(ionres));
				}
			}

		}

		end = clock();
		t = 1000 - (end - start);
		if (t < 0)
		{
			t = 980;
		}
		memset(buff, 0, 8192);
		Sleep(t);
		
	}
	
	
	/*FILE* fp;
	fp = fopen("C:\\Users\\Administrator\\Desktop\\professional Files\\卫星导航课程设计2\\20160620.bin.rtcm", "rb");
	if (fp == NULL)
	{
		printf("CAN NOT OPEN FILE.\n");
		return 0;
	}
	else printf("File Open Successfully.\n");
	fread(buff2, buff_max, 1, fp);
	while (!feof(fp))
	{
        DecodeRTCMData(buff2,&rtcmres, buff_max);
	}*/
	
	fclose(Fout1);
	fclose(Fout2);





	return 0;
}
