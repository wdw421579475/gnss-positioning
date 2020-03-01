#include "stdafx.h"
#include "math.h"
#include "Calculate.h"
#include "DecodeNovData.h"
#include "CordConvert.h"
#include "Matrix.h"
/*
返回值：
1：未读到卫星数据
2：星历过期
*/
int SatPosVelCal(GPSEphem Eph[], XYZ SatPos[], Velocity Vsat[], Obsdata *obs, int i)
{
	if (Eph[i].PRN < 0 || Eph[i].PRN > 32)
	{
		return 1;
	}
	if (obs->obsPRN[i].PRN < 0 || obs->obsPRN[i].PRN > 32)
	{
		return 1;
	}
	int j = 0;
	double n0, n;//平均运动角速度(改正后)
	double Tk;//相对星历参考历元时间
	GPSTime t;//信号发射时刻
	double Mk;//平近点角
	double Ek, E0;//偏近点角
	double Vk;//真近点角
	double faik, duk, uk;//(改正后)升交角距
	double rk0, drk, rk;//(改正后)向径
	double ik0, dik, ik;//(改正后)轨道倾角
	double xk, yk;//卫星在轨道平面的位置
	double omegak;//改正后升交点经度

	double F;
	double dT;

	double Ek1;//偏近点角Ek一阶导数
	double faik1, uk1;//升交角距一阶导数
	double rk1;//向径一阶导数
	double ik1;//轨道倾角一阶导数
	double omegak1;//升交点经度一阶导数
	double R[12];//转化矩阵
	double M[4];//计算矩阵
	double V[3];//速度矩阵
	double xk1, yk1;//轨道上位置与速度
	double dtSV = 0;//设置钟差初值进行两次迭代计算
	double dtr_o, dtSV_o;//用于计算钟速

	double rE;//地球自转角度
	double A[3], B[3], C[9];//用于地球自转改正矩阵
	while (j<2)
	{

		//计算平均运动角速度
		n0 = sqrt(miu / (Eph[i].A*Eph[i].A*Eph[i].A));


		//计算相对于星历参考历元的时间
		t.SecOfWeek = obs->ObsTime.SecOfWeek - obs->obsPRN[i].psr / c - dtSV;
		t.Week = obs->ObsTime.Week;
		while (t.SecOfWeek < 0)
		{
			t.SecOfWeek += 604800;
			t.Week = t.Week - 1;
			if (t.SecOfWeek < -1e-10)
			{
				break;
			}
		}
		Tk = t.SecOfWeek - Eph[i].TOE.SecOfWeek + 604800 * (t.Week - Eph[i].TOE.Week);  /*  前者为信号发射时刻  */
																						/*if (fabs(Tk) > 7200)//若相对时间大于2h，星历过期
																						{
																						return 2;
																						}*/
																						//对平均运动角速度进行改正
		n = n0 + Eph[i].deltaN;

		//计算平近点角、偏近点角、真近点角
		Mk = Eph[i].M0 + n*Tk;

		E0 = Mk;
		while (j<10000)
		{
			Ek = Mk + Eph[i].ecc*sin(E0);
			if (fabs(Ek - E0) <= 1.0e-15)
			{
				break;
			}
			E0 = Ek;
			j++;
		}

		Vk = atan2(sqrt(1 - Eph[i].ecc*Eph[i].ecc)*sin(Ek) / (1 - Eph[i].ecc*cos(Ek)), (cos(Ek) - Eph[i].ecc) / (1 - Eph[i].ecc*cos(Ek)));
		//计算升交角距
		faik = Vk + Eph[i].omega;
		//计算二阶调和改正数
		duk = Eph[i].cus*sin(2 * faik) + Eph[i].cuc*cos(2 * faik);
		drk = Eph[i].crs*sin(2 * faik) + Eph[i].crc*cos(2 * faik);
		dik = Eph[i].cis*sin(2 * faik) + Eph[i].cic*cos(2 * faik);
		//计算改正后的升交角距、向径、轨道倾角
		uk = faik + duk;
		rk = Eph[i].A*(1 - Eph[i].ecc*cos(Ek)) + drk;
		ik = Eph[i].I0 + dik + Eph[i].I_o*Tk;

		//计算卫星在轨道平面上的位置
		xk = rk*cos(uk);
		yk = rk*sin(uk);
		//计算改正后的升交点经度
		omegak = Eph[i].omega0 + (Eph[i].omega_o - omegaE)*Tk - omegaE*Eph[i].TOE.SecOfWeek;
		//计算卫星在地固坐标系下的位置
		SatPos[i].x = xk*cos(omegak) - yk*cos(ik)*sin(omegak);
		SatPos[i].y = xk*sin(omegak) + yk*cos(ik)*cos(omegak);
		SatPos[i].z = yk*sin(ik);
		//对卫星位置进行地球自转改正
		rE = obs->obsPRN[i].psr / c * omegaE;
		C[0] = cos(rE);
		C[1] = sin(rE);
		C[2] = 0;
		C[3] = -sin(rE);
		C[4] = cos(rE);
		C[5] = 0;
		C[6] = 0;
		C[7] = 0;
		C[8] = 1;
		A[0] = SatPos[i].x;
		A[1] = SatPos[i].y;
		A[2] = SatPos[i].z;
		matrix_mult(3, 3, 3, 1, C, A, B);
		SatPos[i].x = B[0];
		SatPos[i].y = B[1];
		SatPos[i].z = B[2];
		//**********计算钟差************//
		F = -2 * sqrt(miu) / (c*c);
		dT = F * Eph[i].ecc*sqrt(Eph[i].A)*sin(Ek);
		dtSV = Eph[i].af0 + Eph[i].af1*(t.SecOfWeek - Eph[i].TOC.SecOfWeek + 604800 * (t.Week - Eph[i].TOC.Week)) + Eph[i].af2
			*(t.SecOfWeek - Eph[i].TOC.SecOfWeek + 604800 * (t.Week - Eph[i].TOC.Week))*(t.SecOfWeek - Eph[i].TOC.SecOfWeek + 604800 * (t.Week - Eph[i].TOC.Week)) + dT - Eph[i].tgd;
		//Vsat[i].deltaTsv = Eph[i].af0 + Eph[i].af1*(t.SecOfWeek - Eph[i].TOC.SecOfWeek) + Eph[i].af2*(pow((t.SecOfWeek - Eph[i].TOC.SecOfWeek), 2)) + dT - Eph[i].tgd;
		Vsat[i].deltaTsv = dtSV;
		//if (fabs(Vsat[i].deltaTsv) > 1)//计算结果中部分出现得到的钟差过大，将错误的结果置零
		//{
		//	Vsat[i].deltaTsv = 0;
		//}

		//**********计算卫星速度***********//
		//计算Ek导数
		Ek1 = Eph[i].N / (1 - Eph[i].ecc*cos(Ek));
		//计算uk导数
		faik1 = sqrt((1 + Eph[i].ecc) / (1 - Eph[i].ecc))*cos(Vk / 2)*cos(Vk / 2)*Ek1 / (cos(Ek / 2)*cos(Ek / 2));
		uk1 = 2 * (Eph[i].cus*cos(2 * faik) - Eph[i].cuc*sin(2 * faik))*faik1 + faik1;
		//计算rk导数
		rk1 = Eph[i].A*Eph[i].ecc*sin(Ek)*Ek1 + 2 * (Eph[i].crs*cos(2 * faik) - Eph[i].crc*sin(2 * faik))*faik1;
		//计算ik导数
		ik1 = Eph[i].I_o + 2 * (Eph[i].cis*cos(2 * faik) - Eph[i].cic*sin(2 * faik))*faik1;
		//计算欧米伽k导数
		omegak1 = Eph[i].omega_o - omegaE;
		//计算转化矩阵R
		R[0] = cos(omegak);
		R[1] = -sin(omegak)*cos(ik);
		R[2] = -(xk*sin(omegak) + yk*cos(omegak)*cos(ik));
		R[3] = yk*sin(omegak)*sin(ik);
		R[4] = sin(omegak);
		R[5] = cos(omegak)*cos(ik);
		R[6] = xk*cos(omegak) - yk*sin(omegak)*cos(ik);
		R[7] = yk*cos(omegak)*sin(ik);
		R[8] = 0;
		R[9] = sin(ik);
		R[10] = 0;
		R[11] = yk*cos(ik);
		//求计算矩阵
		xk1 = rk1*cos(uk) - rk*uk1*sin(uk);
		yk1 = rk1*sin(uk) + rk*uk1*cos(uk);
		M[0] = xk1;
		M[1] = yk1;
		M[2] = omegak1;
		M[3] = ik1;
		//矩阵相乘得到卫星速度
		matrix_mult(3, 4, 4, 1, R, M, V);
		Vsat[i].vx = V[0];
		Vsat[i].vy = V[1];
		Vsat[i].vz = V[2];
		//卫星速度地球自转改正
		A[0] = Vsat[i].vx;
		A[1] = Vsat[i].vy;
		A[2] = Vsat[i].vz;
		matrix_mult(3, 3, 3, 1, C, A, B);
		Vsat[i].vx = B[0];
		Vsat[i].vy = B[1];
		Vsat[i].vz = B[2];

		//卫星钟速计算
		dtr_o = F * Eph[i].ecc*sqrt(Eph[i].A)*cos(Ek)*Ek1;
		dtSV_o = Eph[i].af1 + 2 * Eph[i].af2*(t.SecOfWeek - Eph[i].TOC.SecOfWeek + 604800 * (t.Week - Eph[i].TOC.Week)) + dtr_o;
		Vsat[i].clkvel = dtSV_o;

		j++;


	}


	return 0;
}


