#include "stdafx.h"
#include "math.h"
#include "ErrorCorrect.h"
#include "Calculate.h"
#include "DecodeNovData.h"
#include "CordConvert.h"
#include "Matrix.h"
#include "spp.h"

#define L1_GPS  0.19029367279836488047631742646405  //多普勒系数
/*返回值
1：未读到卫星数据
*/

int sppposition(GPSEphem Eph[], Obsdata *obs, Velocity Vsat[], XYZ position[], PositionRESULT *posres, sppresult *res, IONO *ion)
{
	int i;
	int m;
	int n;
	int k = 0;
	int qq;
	int sn = 0;//读取到卫星数
	int numofsat = 0;//用于统计观测数据中观测到的卫星个数
	int j = 0;
	double xx = 0;//存放迭代距离更新量，判定迭代条件

	XYZ xyzpos1;
	XYZ xyzpos2;
	BLH blhpos1;
	BLH blhpos2;
	GPSTime tt;//信号发射时刻
	double ELE, AZI;//高度角，方位角
	double dTrop[32];//对流层延迟
	double IONDELAY[32];//电离层延迟
	double deltaT;//信号传播时间
	double B[128], BT[128], w[32];//最小二乘求解矩阵
	double DJ[1024] = { 0 };//对角矩阵
	double P1[128], P2[128], P3[128];//用于矩阵乘法中传递中间结果
	double Q[16], QI[16];
	double x[4];//存放最小二乘结果

	double v[32], vT[32];
	double V[1];
	//double sigma;//精度评定

	double ps[32];//用于存储迭代过程中接收机位置和卫星信号发射时刻位置的距离

				  //数据预处理
	xyzpos1.x = 1;
	xyzpos1.y = 1;
	xyzpos1.z = 1;//设置接收机位置初值为(1,1,1)
	res->xyzspppos.x = xyzpos1.x;
	res->xyzspppos.y = xyzpos1.y;
	res->xyzspppos.z = xyzpos1.z;
	res->dp = 0;
	res->dt = 0;//迭代初值

	while (k < 100 || fabs(xx) > 1)
	{
		sn = 0;//每次循环将卫星数置零
			   //计算对流层电离层延迟
		for (i = 0;i < 32;i++)
		{

			if (obs->obsPRN[i].PRN < 0 || obs->obsPRN[i].PRN > 32 || Eph[i].PRN < 0 || Eph[i].PRN > 32)
			{
				continue;//无效卫星数据，继续下一个循环
			}
			else
			{
				xyzpos2.x = position[i].x;
				xyzpos2.y = position[i].y;
				xyzpos2.z = position[i].z;

				xyzpos1.x = res->xyzspppos.x;
				xyzpos1.y = res->xyzspppos.y;
				xyzpos1.z = res->xyzspppos.z;

				XYZtoBLH(&xyzpos1, &blhpos1);
				XYZtoBLH(&xyzpos2, &blhpos2);//将坐标转换为经纬高

											

				ELE = atan2(cos(blhpos2.longitude - blhpos1.longitude)*cos(blhpos1.latitude) - 0.1513, sqrt(1 - cos(blhpos2.longitude - blhpos1.longitude)*cos(blhpos1.latitude)*cos(blhpos2.longitude - blhpos1.longitude)*cos(blhpos1.latitude)));
				AZI = atan2(tan(blhpos2.longitude - blhpos1.longitude), sin(blhpos1.latitude));
				dTrop[i] = Hopfield(blhpos1.h, ELE);

				deltaT = sqrt((xyzpos2.x - xyzpos1.x)*(xyzpos2.x - xyzpos1.x) + (xyzpos2.y - xyzpos1.y)*(xyzpos2.y - xyzpos1.y) + (xyzpos2.z - xyzpos1.z)*(xyzpos2.z - xyzpos1.z)) / c;
				tt.SecOfWeek = obs->ObsTime.SecOfWeek - obs->obsPRN[i].psr / c - Vsat[i].deltaTsv + deltaT;
				tt.Week = obs->ObsTime.Week;
				if (tt.SecOfWeek < 0)
				{
					tt.SecOfWeek += 604800;
					tt.Week = tt.Week - 1;
				}
				IONDELAY[i] = Klobutchar(ion, &blhpos1, &tt, ELE, AZI);

			}
			ps[i] = sqrt((xyzpos2.x - xyzpos1.x)*(xyzpos2.x - xyzpos1.x) + (xyzpos2.y - xyzpos1.y)*(xyzpos2.y - xyzpos1.y) + (xyzpos2.z - xyzpos1.z)*(xyzpos2.z - xyzpos1.z));
		}

		//观测方程线性化，计算B，w矩阵
		for (j = 0; j < 32; j++)
		{
			if (Eph[j].PRN == obs->obsPRN[j].PRN && Eph[j].PRN > 0 && Eph[j].PRN < 33)
			{
				w[sn] = obs->obsPRN[j].psr - ps[j] - res->dp + c * Vsat[j].deltaTsv - dTrop[j] - c*IONDELAY[j];
				B[4 * sn] = (res->xyzspppos.x - position[j].x) / ps[j];
				B[4 * sn + 1] = (res->xyzspppos.y - position[j].y) / ps[j];
				B[4 * sn + 2] = (res->xyzspppos.z - position[j].z) / ps[j];
				B[4 * sn + 3] = 1;
				sn++;
			}

		}
		//构建对角矩阵
		for (n = 0;n < sn;n++)
		{
			DJ[n * sn + n] = 1;
		}

		//最小二乘求解
		if (sn >= 4)
		{
			matrix_trans(sn, 4, B, BT);
			matrix_mult(4, sn, sn, sn, BT, DJ, P1);
			matrix_mult(4, sn, sn, 4, P1, B, Q);
			for (i = 0;i < 16;i++)
			{
				if (fabs(Q[i]) > 1e5)
				{
					continue;
				}
			}

			qq = MatrixInv(4, Q, QI);
			if (qq != 1)
			{
				return 0;
			}

			matrix_mult(4, 4, 4, sn, QI, BT, P2);
			matrix_mult(4, sn, sn, sn, P2, DJ, P3);
			matrix_mult(4, sn, sn, 1, P3, w, x);

			xx = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);//更新距离

			res->xyzspppos.x += x[0];
			res->xyzspppos.y += x[1];
			res->xyzspppos.z += x[2];
			res->dp += x[3];
			res->dt = res->dp / c;

			//精度评定
			for (m = 0; m < sn; m++)
			{
				v[m] = B[4 * m] * res->xyzspppos.x + B[4 * m + 1] * res->xyzspppos.y + B[4 * m + 2] * res->xyzspppos.z + B[4 * m + 3] * res->dp - w[m];
			}
			matrix_trans(sn, 1, v, vT);
			matrix_mult(1, sn, sn, 1, vT, v, V);
			res->sigma = sqrt(V[0] / (sn - 4));
			res->P_Dop = sqrt(QI[0] + QI[5] + QI[10] + QI[15]);
			res->P_Pdop = sqrt(QI[0] + QI[5] + QI[10]);

			k++;

		}

	}
	XYZtoBLH(&res->xyzspppos, &res->blhspppos);//将结果转换为经纬高


	return 0;
}

int sppvelocity(GPSEphem Eph[], Velocity Vsat[], Obsdata *obs, XYZ position[], sppresult *res, velresult *vres)
{
	int i, j, k, m;
	int qq;
	int sn = 0;//观测卫星数
	double B[128], BT[128];
	double w[32];
	double ps[32], ps_o[32];//卫星与接收机距离及其一阶导数
	double Q[16], IQ[16];
	double DJ[1024] = { 0 };
	double P1[128], P2[128], P3[128];
	double vel[4];
	double v[32], vT[32];
	double V[1];
	XYZ xyzpos;

	sn = 0;//观测卫星数置零
	for (i = 0;i < 32;i++)
	{


		if (Eph[i].PRN == obs->obsPRN[i].PRN && obs->obsPRN[i].PRN > 0 && obs->obsPRN[i].PRN < 33)
		{
			xyzpos.x = position[i].x;
			xyzpos.y = position[i].y;
			xyzpos.z = position[i].z;
			ps[i] = sqrt((xyzpos.x - res->xyzspppos.x)*(xyzpos.x - res->xyzspppos.x) + (xyzpos.y - res->xyzspppos.y)*(xyzpos.y - res->xyzspppos.y) + (xyzpos.z - res->xyzspppos.z)*(xyzpos.z - res->xyzspppos.z));
			ps_o[i] = ((xyzpos.x - res->xyzspppos.x)*Vsat[i].vx + (xyzpos.y - res->xyzspppos.y)*Vsat[i].vy + (xyzpos.z - res->xyzspppos.z)*Vsat[i].vz) / ps[i];


		}
		else
		{
			continue;//无效卫星数据，继续下一个循环
		}
	}
	for (i = 0; i < 32; i++)
	{
		if (Eph[i].PRN == obs->obsPRN[i].PRN && obs->obsPRN[i].PRN > 0 && obs->obsPRN[i].PRN < 33)
		{
			w[sn] = obs->obsPRN[i].DopplerShift*(-L1_GPS) - ps_o[i] + c * Vsat[i].clkvel;
			B[4 * sn] = (res->xyzspppos.x - position[i].x) / ps[i];
			B[4 * sn + 1] = (res->xyzspppos.y - position[i].y) / ps[i];
			B[4 * sn + 2] = (res->xyzspppos.z - position[i].z) / ps[i];
			B[4 * sn + 3] = 1;
			sn++;
		}

	}
	for (j = 0; j < sn; j++)
	{
		DJ[j * sn + j] = 1;
	}

	if (sn > 4)
	{
		matrix_trans(sn, 4, B, BT);
		matrix_mult(4, sn, sn, sn, BT, DJ, P1);
		matrix_mult(4, sn, sn, 4, P1, B, Q);

		for (i = 0;i < 16;i++)
		{
			if (fabs(Q[i]) > 1e5)
			{
				continue;
			}
		}
		qq = MatrixInv(4, Q, IQ);
		if (qq != 1)
		{
			return 0;
		}

		matrix_mult(4, 4, 4, sn, IQ, BT, P2);
		matrix_mult(4, sn, sn, sn, P2, DJ, P3);
		matrix_mult(4, sn, sn, 1, P3, w, vel);

		vres->vx = vel[0];
		vres->vy = vel[1];
		vres->vz = vel[2];
		vres->dp_o = vel[3];
		vres->dt_o = vres->dp_o / c;

		for (k = 0; k < sn; k++)
		{
			v[k] = B[4 * k] * vres->vx + B[4 * k + 1] * vres->vy + B[4 * k + 2] * vres->vz + B[4 * k + 3] * vres->dp_o - w[k];
		}

		matrix_trans(sn, 1, v, vT);
		matrix_mult(1, sn, sn, 1, vT, v, V);
		vres->sigma = sqrt(V[0] / (sn - 4));
		vres->V_Dop = sqrt(IQ[0] + IQ[5] + IQ[10] + IQ[15]);
		vres->V_Pdop = sqrt(IQ[0] + IQ[5] + IQ[10]);
	}
	return 0;

}

int dspp(GPSEphem Eph[], Obsdata *obs, Velocity Vsat[], XYZ position[], PositionRESULT *posres, sppresult *res, RTCMRES *rtcmres)
{
	int i;
	int m;
	int n;
	int k = 0;
	int qq;
	int sn = 0;//读取到卫星数
	int numofsat = 0;//用于统计观测数据中观测到的卫星个数
	int j = 0;
	double xx = 0;//存放迭代距离更新量，判定迭代条件

	XYZ xyzpos1;
	XYZ xyzpos2;
	BLH blhpos1;
	BLH blhpos2;
	GPSTime tt;//信号发射时刻
	//double ELE, AZI;//高度角，方位角
	//double dTrop[32];//对流层延迟
	//double IONDELAY[32];//电离层延迟
	double deltaT;//信号传播时间
	double B[128], BT[128], w[32];//最小二乘求解矩阵
	double DJ[1024] = { 0 };//对角矩阵
	double P1[128], P2[128], P3[128];//用于矩阵乘法中传递中间结果
	double Q[16], QI[16];
	double x[4];//存放最小二乘结果

	double v[32], vT[32];
	double V[1];
	//double sigma;//精度评定

	double ps[32];//用于存储迭代过程中接收机位置和卫星信号发射时刻位置的距离

				  //数据预处理
	xyzpos1.x = 1;
	xyzpos1.y = 1;
	xyzpos1.z = 1;//设置接收机位置初值为(1,1,1)
	res->xyzspppos.x = xyzpos1.x;
	res->xyzspppos.y = xyzpos1.y;
	res->xyzspppos.z = xyzpos1.z;
	res->dp = 0;
	res->dt = 0;//迭代初值

	while (k < 100 || fabs(xx) > 1)
	{
		sn = 0;//每次循环将卫星数置零
			   //计算对流层电离层延迟
		for (i = 0;i < 32;i++)
		{
			if (obs->obsPRN[i].PRN < 0 || obs->obsPRN[i].PRN > 32 || Eph[i].PRN < 0 || Eph[i].PRN > 32)
			{
				continue;//无效卫星数据，继续下一个循环
			}
			else
			{
				xyzpos2.x = position[i].x;
				xyzpos2.y = position[i].y;
				xyzpos2.z = position[i].z;

				xyzpos1.x = res->xyzspppos.x;
				xyzpos1.y = res->xyzspppos.y;
				xyzpos1.z = res->xyzspppos.z;

				XYZtoBLH(&xyzpos1, &blhpos1);
				XYZtoBLH(&xyzpos2, &blhpos2);//将坐标转换为经纬高
			}
			ps[i] = sqrt((xyzpos2.x - xyzpos1.x)*(xyzpos2.x - xyzpos1.x) + (xyzpos2.y - xyzpos1.y)*(xyzpos2.y - xyzpos1.y) + (xyzpos2.z - xyzpos1.z)*(xyzpos2.z - xyzpos1.z));
		}

		//观测方程线性化，计算B，w矩阵
		for (j = 0; j < 32; j++)
		{
			if (rtcmres->msg1[j].PRN == obs->obsPRN[j].PRN &&rtcmres->msg1[j].PRN > 0 && rtcmres->msg1[j].PRN < 33 )
			{
				if (Eph[j].PRN == obs->obsPRN[j].PRN && fabs(obs->obsPRN[j].psr) < 1e10 && rtcmres->msg1[j].IOD == Eph[j].IODE1)
				{
					w[sn] = obs->obsPRN[j].psr - ps[j] - res->dp + c * Vsat[j].deltaTsv + rtcmres->msg1[j].PRC + rtcmres->msg1[j].RRC*(int(obs->ObsTime.SecOfWeek) % 3600 - rtcmres->head.modZ);
					B[4 * sn] = (res->xyzspppos.x - position[j].x) / ps[j];
					B[4 * sn + 1] = (res->xyzspppos.y - position[j].y) / ps[j];
					B[4 * sn + 2] = (res->xyzspppos.z - position[j].z) / ps[j];
					B[4 * sn + 3] = 1;
					sn++;
				}
			}
			

		}
		if (sn < 4)
		{
			return 0;
		}
		//构建对角矩阵
		for (n = 0;n < sn;n++)
		{
			DJ[n * sn + n] = 1;
		}

		//最小二乘求解
		if (sn >= 4)
		{
			matrix_trans(sn, 4, B, BT);
			matrix_mult(4, sn, sn, sn, BT, DJ, P1);
			matrix_mult(4, sn, sn, 4, P1, B, Q);
			for (i = 0;i < 16;i++)
			{
				if (fabs(Q[i]) > 1e5)
				{
					continue;
				}
			}

			qq = MatrixInv(4, Q, QI);
			if (qq != 1)
			{
				return 0;
			}

			matrix_mult(4, 4, 4, sn, QI, BT, P2);
			matrix_mult(4, sn, sn, sn, P2, DJ, P3);
			matrix_mult(4, sn, sn, 1, P3, w, x);

			xx = sqrt(x[0] * x[0] + x[1] * x[1] + x[2] * x[2]);//更新距离

			res->xyzspppos.x += x[0];
			res->xyzspppos.y += x[1];
			res->xyzspppos.z += x[2];
			res->dp += x[3];
			res->dt = res->dp / c;

			//精度评定
			for (m = 0; m < sn; m++)
			{
				v[m] = B[4 * m] * res->xyzspppos.x + B[4 * m + 1] * res->xyzspppos.y + B[4 * m + 2] * res->xyzspppos.z + B[4 * m + 3] * res->dp - w[m];
			}
			matrix_trans(sn, 1, v, vT);
			matrix_mult(1, sn, sn, 1, vT, v, V);
			res->sigma = sqrt(V[0] / (sn - 4));
			res->P_Dop = sqrt(QI[0] + QI[5] + QI[10] + QI[15]);
			res->P_Pdop = sqrt(QI[0] + QI[5] + QI[10]);

			k++;

		}

	}
	XYZtoBLH(&res->xyzspppos, &res->blhspppos);//将结果转换为经纬高


	return 0;
}