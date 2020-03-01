#include "stdafx.h"
#include "math.h"
#include "RTCMDecode.h"
#include "DecodeNovData.h"
#define MAXLEN 1024

int DecodeRTCMData(unsigned char *buff2, RTCMRES *rtcmresult,int len)
{
	int i, j;
	int mark;
	int n;
	//unsigned char buff[MAXLEN];
	//unsigned char w[3];
	unsigned char raw[5], rot[5], w[4], D29, D30;
	unsigned char W[200] = { 0 };
	RTCMBUFF RTCMdat;
	memset(&RTCMdat, 0, sizeof(RTCMBUFF));
	memset(rtcmresult, 0, sizeof(RTCMRES));
	RTCMdat.buff_len = len;
	RTCMdat.buff_position = 0;
    //将缓冲区的数据存进buff结构体内，对数据结构体进行处理
	for (i = 0;i < len;i++)
	{
		RTCMdat.buff2[i] = buff2[i];
	}
	//D29，D30初值设为1
	
	while (RTCMdat.buff_position < len - 5)
	{
		D29 = 1;
		D30 = 1;
		//开始字节扫描，找到有效数据开始存储
		n = RTCMdat.buff_position;
		for (i = 0;i < 5;)
		{
			
			raw[i] = RTCMdat.buff2[n + i];
			
			//查找有效字节
			if (raw[i] >> 7 == 0 && (raw[i] >> 6 & 0x01) == 1)
			{
				rot[i] = RTCMroll(raw[i]);//字节滚动
				i++;
			}
			else
			{
				n++;
			}
		}
		RTCMdat.buff_position++;
		//字节合并
		FormatRotToWords(rot, w);
		//字节取反码
		if (D30 == 1)
		{
			for (i = 0;i < 3;i++)
			{
				w[i] = 256 - w[i] - 1;
			}
		}
		//字节同步，找到同步标识符0x66

		mark = PreambleSearch(raw, w, &RTCMdat, &D29, &D30, W);
		if (mark == 1)//如果当前找到同步标识符并奇偶校验通过则开始解码
		{
			GetRTCMHeadAndResultFromBuff(W, &D29, &D30, rtcmresult);
		}
		else
		{ 
			continue; 
		}
	}
	
	return 0;
}

//搜索引导字函数
int PreambleSearch(unsigned char raw[], unsigned char word[], RTCMBUFF *rtcmdata, unsigned char *D29, unsigned char *D30, unsigned char W[])
{
	int i, P, P2;
	int j, n;
	int length;//一组数据长度
	
	unsigned char rawhead2[5], wordhead2[4], rothead2[5];
	
	if (rtcmdata->buff_position + 4 >= rtcmdata->buff_len) return 0;//越界，错误返回0
	
	if (word[0] == 0x66)
	{
		*D29 = 1;
		*D30 = 1;//先设为1
		//奇偶校验
		P = RTCMParity(word, *D29, *D30);
		if (P != 0)//第一个字奇偶校验通过，组成第二个字并奇偶校验
		{
			*D30 = word[3] & 0x01;
			*D29 = word[3] >> 1 & 0x01;
			j = rtcmdata->buff_position + 4;
			for (i = 0;i < 5;i++)//将第二个头存储进数组
			{
				rawhead2[i] = rtcmdata->buff2[j + i];
			}
			for (i = 0;i < 5;i++)//翻滚
			{
				rothead2[i] = RTCMroll(rawhead2[i]);
			}
			FormatRotToWords(rothead2, wordhead2);
			if (*D30 == 1)
			{
				for (i = 0;i < 4;i++)
				{
					wordhead2[i] = 256 - wordhead2[i] - 1;
				}
			}
			//对第二个字奇偶校验，通过则存储该电文
			P2 = RTCMParity(wordhead2, *D29, *D30);
			if (P2 != 0)//第二个字奇偶校验通过
			{
				//*D30 = wordhead2[3] & 0x01;
				//*D29 = wordhead2[3] >> 1 & 0x01;//更新D29，D30
				length = wordhead2[2] >> 3;
				for (i = 0;i < 5 * (length + 2);i++)
				{
					//n = rtcmdata->buff_position;
					W[i] = rtcmdata->buff2[(rtcmdata->buff_position++) - 1];
				}
				
				
                return 1;
			}
			else return 0;
			
		}
		else return 0;
		
	}
	else
	{
		return 0;
	}

	

	return 0;
}
//RTCM校验计算


bool GetRTCMHeadAndResultFromBuff(unsigned char W[], unsigned char *D29, unsigned char *D30, RTCMRES *rtcmres)
{
	int i, j;
	int k = 0;
	int Satid;//卫星ID号
	int P;
	unsigned char word[4], neww[100], head1[4], head2[4], raw[5], rot[5], res[5];
	for (i = 0;i < 5;i++)//算第一个头字
	{
		raw[i] = W[i];
		rot[i] = RTCMroll(raw[i]);
	}
	FormatRotToWords(rot, head1);
	for (i = 0;i < 3;i++)//取反码
	{
		head1[i] = 256 - head1[i] - 1;
	}
	
	//第二个头字
	for (i = 0;i < 5;i++)
	{
		raw[i] = W[i+5];
		rot[i] = RTCMroll(raw[i]);
	}
	FormatRotToWords(rot, head2);
	if (*D30 == 1)
	{
		for (i = 0;i < 3;i++)
		{
			head2[i] = 256 - head2[i] - 1;
		}
	}
	rtcmres->head.preamble = head1[0];
	rtcmres->head.MSGType = head1[1] >> 2;
	rtcmres->head.REFSTN_ID = head1[2];
	rtcmres->head.modZ = 0.6 * (head2[0] * 32 + (head2[1] >> 3));
	rtcmres->head.SeqNo = head2[1] & 0x07;
	rtcmres->head.LEN = head2[2] >> 3;
	rtcmres->head.STNhealth = head2[2] & 0x07;
	rtcmres->head.SN = int(rtcmres->head.LEN / 5) * 3 + (rtcmres->head.LEN % 5) / 2;
	*D30 = head2[3] & 0x01;
	*D29 = head2[3] >> 1 & 0x01;
	//解码电文数据
	while (k < rtcmres->head.LEN)
	{
		for (i = 0;i < 5;i++)
		{
			raw[i] = W[i + 10 + 5 * k];
			rot[i] = RTCMroll(raw[i]);
		}
		FormatRotToWords(rot, word);
		if (*D30 == 1)
		{
			for (i = 0;i < 3;i++)
			{
				word[i] = 256 - word[i] - 1;
			}
		}
		//奇偶校验
		P = RTCMParity(word, *D29, *D30);
		if (P != 0)//校验通过
		{
			for (i = 0;i < 3;i++)
			{
				neww[i + k * 3] = word[i];
			}
			//更新D29D30
			*D30 = word[3] & 0x01;
			*D29 = word[3] >> 1 & 0x01;
		}
		else
		{
			memset(&rtcmres->head,0,sizeof(rtcmres->head));
			memset(&rtcmres->msg1, 0, sizeof(rtcmres->msg1));
			return false;
		}
		k++;
	}
	//printf("%lf,%d,%d\n", rtcmres->head.modZ, rtcmres->head.LEN, rtcmres->head.SN);
	//开始解码
	for (i = 0;i < rtcmres->head.SN;i++)
	{
		for (j = 0;j < 5;j++)
		{
			res[j] = neww[j + 5 * i];
		}
		Satid = res[0] & 0x1f;
		if (Satid == 0)
		{
			Satid = 32;
		}
		rtcmres->msg1[Satid - 1].PRN = Satid;
		rtcmres->msg1[Satid - 1].S = res[0] >> 7;
		rtcmres->msg1[Satid - 1].UDRE = res[0] >> 5 & 0x03;

		short m = (neww[5 * i + 1] << 8);
		m = m | neww[5 * i + 2];
		char a = neww[5 * i + 3];

		if (rtcmres->msg1[Satid - 1].S == 0)
		{
			rtcmres->msg1[Satid - 1].PRC = 0.02 * m;
			rtcmres->msg1[Satid - 1].RRC = a * 0.002;
		}
		else
		{
			rtcmres->msg1[Satid - 1].PRC = 0.32 * m;
			rtcmres->msg1[Satid - 1].RRC = a * 0.032;
		}
		rtcmres->msg1[Satid - 1].IOD = res[4];
		//fprintf(Fout3,"%d %f %f %f\n", rtcmres->msg1[Satid - 1].PRN, rtcmres->msg1[Satid - 1].PRC, rtcmres->msg1[Satid - 1].RRC, rtcmres->msg1[Satid - 1].IOD);
		
	}
	
	


	return true;
}

bool RTCMParity(unsigned char w[], unsigned char D29, unsigned char D30)
{
	unsigned char d[6] = { 0 };
	unsigned char res = 0;

	d[0] = D29 ^ (w[0] >> 7 & 0x01) ^ (w[0] >> 6 & 0x01) ^ (w[0] >> 5 & 0x01) ^ (w[0] >> 3 & 0x01) ^ (w[0] >> 2 & 0x01) ^
		(w[1] >> 6 & 0x01) ^ (w[1] >> 5 & 0x01) ^ (w[1] >> 4 & 0x01) ^ (w[1] >> 3 & 0x01) ^ (w[1] >> 2 & 0x01) ^
		(w[2] >> 7 & 0x01) ^ (w[2] >> 6 & 0x01) ^ (w[2] >> 4 & 0x01) ^ (w[2] >> 1 & 0x01);

	d[1] = D30 ^ (w[0] >> 6 & 0x01) ^ (w[0] >> 5 & 0x01) ^ (w[0] >> 4 & 0x01) ^ (w[0] >> 2 & 0x01) ^ (w[0] >> 1 & 0x01) ^
		(w[1] >> 5 & 0x01) ^ (w[1] >> 4 & 0x01) ^ (w[1] >> 3 & 0x01) ^ (w[1] >> 2 & 0x01) ^ (w[1] >> 1 & 0x01) ^
		(w[2] >> 6 & 0x01) ^ (w[2] >> 5 & 0x01) ^ (w[2] >> 3 & 0x01) ^ (w[2] & 0x01);

	d[2] = D29 ^ (w[0] >> 7 & 0x01) ^ (w[0] >> 5 & 0x01) ^ (w[0] >> 4 & 0x01) ^ (w[0] >> 3 & 0x01) ^ (w[0] >> 1 & 0x01) ^ (w[0] & 0x01) ^
		(w[1] >> 4 & 0x01) ^ (w[1] >> 3 & 0x01) ^ (w[1] >> 2 & 0x01) ^ (w[1] >> 1 & 0x01) ^ (w[1] & 0x01) ^
		(w[2] >> 5 & 0x01) ^ (w[2] >> 4 & 0x01) ^ (w[2] >> 2 & 0x01);

	d[3] = D30 ^ (w[0] >> 6 & 0x01) ^ (w[0] >> 4 & 0x01) ^ (w[0] >> 3 & 0x01) ^ (w[0] >> 2 & 0x01) ^ (w[0] & 0x01) ^
		(w[1] >> 7 & 0x01) ^ (w[1] >> 3 & 0x01) ^ (w[1] >> 2 & 0x01) ^ (w[1] >> 1 & 0x01) ^ (w[1] & 0x01) ^
		(w[2] >> 7 & 0x01) ^ (w[2] >> 4 & 0x01) ^ (w[2] >> 3 & 0x01) ^ (w[2] >> 1 & 0x01);

	d[4] = D30 ^ (w[0] >> 7 & 0x01) ^ (w[0] >> 5 & 0x01) ^ (w[0] >> 3 & 0x01) ^ (w[0] >> 2 & 0x01) ^ (w[0] >> 1 & 0x01) ^
		(w[1] >> 7 & 0x01) ^ (w[1] >> 6 & 0x01) ^ (w[1] >> 2 & 0x01) ^ (w[1] >> 1 & 0x01) ^ (w[1] & 0x01) ^
		(w[2] >> 7 & 0x01) ^ (w[2] >> 6 & 0x01) ^ (w[2] >> 3 & 0x01) ^ (w[2] >> 2 & 0x01) ^ (w[2] & 0x01);

	d[5] = D29 ^ (w[0] >> 5 & 0x01) ^ (w[0] >> 3 & 0x01) ^ (w[0] >> 2 & 0x01) ^ (w[0] & 0x01) ^
		(w[1] >> 7 & 0x01) ^ (w[1] >> 6 & 0x01) ^ (w[1] >> 5 & 0x01) ^ (w[1] >> 3 & 0x01) ^ (w[1] >> 1 & 0x01) ^
		(w[2] >> 5 & 0x01) ^ (w[2] >> 2 & 0x01) ^ (w[2] >> 1 & 0x01) ^ (w[2] & 0x01);

	res = d[5] | (d[4] << 1) | (d[3] << 2) | (d[2] << 3) | (d[1] << 4) | (d[0] << 5);

	if (res == w[3])  return true;
	else return false;
}
//字节滚动函数
unsigned char RTCMroll(unsigned char raw)
{
	unsigned char a, b, c, d, e, f;
	a = raw >> 5 & 0x01;
	b = raw >> 3 & 0x02;
	c = raw >> 1 & 0x04;
	d = raw << 1 & 0x08;
	e = raw << 3 & 0x10;
	f = raw << 5 & 0x20;
	raw = a + b + c + d + e + f;
	return raw;

}
//字节合并函数
void FormatRotToWords(unsigned char *rot, unsigned char *word)
{
	word[0] = (rot[0] << 2) + (rot[1] >> 4);
	word[1] = (rot[1] << 4) + (rot[2] >> 2);
	word[2] = (rot[2] << 6) + rot[3];
	word[3] = rot[4];
}