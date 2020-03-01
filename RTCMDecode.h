#pragma once
#ifndef __RTCMDECODE_H__
#define __RTCMDECODE_H__
#include "TimeConvert.h"
#define MAXLEN 8192
#define MAXCHANNEL 32

struct RTCMHEAD
{
	int preamble;//引导字
	float REFSTN_ID;//基站ID
	unsigned short MSGType;
	double modZ;//改正Z计数
	int SeqNo;//序列号
	unsigned short LEN;//帧长
	unsigned short STNhealth;//基准站健康状态
	unsigned char SN;
};
struct RTCMSG1
{
	unsigned short S;//Scale
	unsigned short PRN;
	unsigned short UDRE;
	double PRC;//伪距改正数
	double RRC;//伪距改正数变化率
	double IOD;//当前所用星历
};
struct RTCMSG3
{
	double ECEF_x;
	double ECEF_y;
	double ECEF_z;

};
struct RTCMRES
{
	GPSTime t;
	RTCMHEAD head;
	RTCMSG1 msg1[MAXCHANNEL];
	RTCMSG3 msg3;
};
struct RTCMBUFF
{
	unsigned char buff2[MAXLEN];
	int buff_position;//处理位置
	int buff_len;//数据长
};


int DecodeRTCMData(unsigned char *buff2, RTCMRES *rtcmresult, int len);
int PreambleSearch(unsigned char raw[], unsigned char word[], RTCMBUFF *rtcmdata, unsigned char *D29, unsigned char *D30, unsigned char W[]);
void FormatRotToWords(unsigned char *rot, unsigned char *word);
unsigned char RTCMroll(unsigned char raw);
bool RTCMParity(unsigned char w[], unsigned char D29, unsigned char D30);
bool GetRTCMHeadAndResultFromBuff(unsigned char W[], unsigned char *D29, unsigned char *D30, RTCMRES *rtcmres);
#endif