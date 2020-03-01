#pragma once
#ifndef __MATRIX_H__
#define __MATRIX_H__


void matrix_add(int row, int line, double matA[], double matB[], double matC[]);
void matrix_minus(int row, int line, double matA[], double matB[], double matC[]);
void matrix_mult(int row1, int line1, int row2, int line2, double matA[], double matB[], double matC[]);
void matrix_trans(int row, int line, double matA[], double matB[]);
int MatrixInv(int n, double a[], double b[]);
void vector_pointmult(int m, double vecA[], double vecB[], double vecC[]);
void vector_crossmult(double vecA[], double vecB[], double vecC[]);


#endif	// MATRIX_H
