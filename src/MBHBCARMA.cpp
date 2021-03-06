#ifdef __INTEL_COMPILER
    #include <mathimf.h>
    #if defined __APPLE__ && defined __MACH__
        #include <malloc/malloc.h>
    #else
        #include <malloc.h>
    #endif
#else
    #include <math.h>
    #include <mm_malloc.h>
#endif
#include <sys/time.h>
#include <limits>
#include <omp.h>
#include <complex>
#include <cmath>
#ifdef __INTEL_COMPILER
    #include <mathimf.h>
#else
    #include <math.h>
#endif
#include <mkl_types.h>
#define MKL_Complex8 std::complex<float>
#define MKL_Complex16 std::complex<double>
#include <mkl.h>
#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <nlopt.hpp>
#include "Constants.hpp"
#include "MBHBCARMA.hpp"

#define MAXPRINT 20
//#define TIMEALL
//#define TIMEPER
//#define TIMEFINE
//#define DEBUG
//#define DEBUG_LNLIKE
//#define DEBUG_FUNC
//#define DEBUG_CHECKMBHBCARMAPARAMS
//#define DEBUG_SETMBHBCARMA
//#define DEBUG_SETMBHBCARMA_C
//#define DEBUG_SOLVEMBHBCARMA_F
//#define DEBUG_SOLVEMBHBCARMA_Q
//#define DEBUG_FUNCTOR
//#define DEBUG_SETMBHBCARMA_DEEP
//#define DEBUG_BURNSYSTEM
//#define WRITE_BURNSYSTEM
//#define DEBUG_SIMULATESYSTEM
//#define DEBUG_OBSSYSTEM
//#define DEBUG_OBS
//#define DEBUG_CTORMBHBCARMA
//#define DEBUG_DTORMBHBCARMA
//#define DEBUG_ALLOCATEMBHBCARMA
//#define DEBUG_DEALLOCATEMBHBCARMA
//#define DEBUG_DEALLOCATEMBHBCARMA_DEEP
//#define DEBUG_RESETSTATE
//#define DEBUG_CALCLNPOSTERIOR
//#define DEBUG_COMPUTELNPRIOR
//#define DEBUG_COMPUTELNLIKELIHOOD
//#define DEBUG_COMPUTEACVF
//#define DEBUG_RTSSMOOTHER


using namespace std;

double kali::calcLnPrior(const vector<double> &x, vector<double>& grad, void *p2Args) {
	/*! Used for computing good regions */

	if (!grad.empty()) {
		#pragma omp simd
		for (int i = 0; i < x.size(); ++i) {
			grad[i] = 0.0;
			}
		}

	int threadNum = omp_get_thread_num();

	kali::LnLikeArgs *ptr2Args = reinterpret_cast<LnLikeArgs*>(p2Args);
	kali::LnLikeArgs Args = *ptr2Args;
	kali::MBHBCARMA *Systems = Args.Systems;
	double LnPrior = 0.0;

	#ifdef DEBUG_CALCLNPRIOR
	printf("calcMBHBCARMALnPosterior - threadNum: %d; Location: ",threadNum);
	#endif

	if (Systems[threadNum].checkMBHBCARMAParams(const_cast<double*>(&x[0])) == 1) {
		LnPrior = 0.0;
		} else {
		LnPrior = -kali::infiniteVal;
		}

	#ifdef DEBUG_CALCPRIOR
	printf("calcMBHBCARMALnPosterior - threadNum: %d; LnPosterior: %f\n", threadNum, LnPosterior);
	fflush(0);
	#endif

	return LnPrior;
	}

double kali::calcLnPrior(double *walkerPos, void *func_args) {
	/*! Used for computing good regions */

	int threadNum = omp_get_thread_num();

	kali::LnLikeArgs *ptr2Args = reinterpret_cast<LnLikeArgs*>(func_args);
	kali::LnLikeArgs Args = *ptr2Args;
	kali::MBHBCARMA* Systems = Args.Systems;
	double LnPrior = 0.0;

	if (Systems[threadNum].checkMBHBCARMAParams(walkerPos) == 1) {

		#ifdef DEBUG_CALCLNPRIOR
		printf("calcLnLike = threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum <  Systems[threadNum].get_r() + Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
			printf("%f ",walkerPos[dimNum]);
			}
		printf("\n");
		printf("calcLnLike - threadNum: %d; System good!\n",threadNum);
		#endif

		LnPrior = 0.0;
		} else {

		#ifdef DEBUG_CALCLNPRIOR
		printf("calcLnLike = threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum <  Systems[threadNum].get_r() + Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
			printf("%f ",walkerPos[dimNum]);
			}
		printf("\n");
		printf("calcLnLike - threadNum: %d; System bad!\n",threadNum);
		#endif

		LnPrior = -kali::infiniteVal;
		}
	return LnPrior;
	}

double kali::calcLnPosterior(const vector<double> &x, vector<double>& grad, void *p2Args) {
	if (!grad.empty()) {
		#pragma omp simd
		for (int i = 0; i < x.size(); ++i) {
			grad[i] = 0.0;
			}
		}

	int threadNum = omp_get_thread_num();

	kali::LnLikeArgs *ptr2Args = reinterpret_cast<LnLikeArgs*>(p2Args);
	kali::LnLikeArgs Args = *ptr2Args;
	kali::LnLikeData *ptr2Data = Args.Data;
	kali::MBHBCARMA *Systems = Args.Systems;
	double LnPrior = 0.0, LnPosterior = 0.0, old_dt = 0.0;

	if (Systems[threadNum].checkMBHBCARMAParams(const_cast<double*>(&x[0])) == 1) {
		old_dt = Systems[threadNum].get_dt();
		Systems[threadNum].setMBHBCARMA(const_cast<double*>(&x[0]));
		Systems[threadNum].solveMBHBCARMA();
		Systems[threadNum].resetState();
		LnPrior = Systems[threadNum].computeLnPrior(ptr2Data);

		#ifdef DEBUG_CALCLNPOSTERIOR
			printf("calcLnPosterior - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum <  Systems[threadNum].get_r() + Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
				printf("%+17.16e ", x[dimNum]);
				}
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; System good!\n",threadNum);
			printf("calcLnPosterior - threadNum: %d; dt: %e\n",threadNum, Systems[threadNum].get_dt());
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; A\n",threadNum);
			Systems[threadNum].printA();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; w\n",threadNum);
			Systems[threadNum].printw();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; expw\n",threadNum);
			Systems[threadNum].printexpw();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; vr\n",threadNum);
			Systems[threadNum].printvr();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; vrInv\n",threadNum);
			Systems[threadNum].printvrInv();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; B\n",threadNum);
			Systems[threadNum].printB();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; C\n",threadNum);
			Systems[threadNum].printC();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; F\n",threadNum);
			Systems[threadNum].printF();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; Q\n",threadNum);
			Systems[threadNum].printQ();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; Sigma\n",threadNum);
			Systems[threadNum].printSigma();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; X\n",threadNum);
			Systems[threadNum].printX();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; P\n",threadNum);
			Systems[threadNum].printP();
			printf("\n");
            printf("calcLnPosterior - threadNum: %d; LnPrior: %e\n",threadNum, LnPrior);
			printf("\n");
			fflush(0);
			fflush(0);
		#endif

		LnPosterior = Systems[threadNum].computeLnLikelihood(ptr2Data) + LnPrior;

		Systems[threadNum].set_dt(old_dt);
		Systems[threadNum].solveMBHBCARMA();
		//Systems[threadNum].resetState();

		#ifdef DEBUG_CALCLNPOSTERIOR
			printf("calcLnPosterior - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum <  Systems[threadNum].get_r() + Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
				printf("%+17.16e ", x[dimNum]);
				}
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; X\n",threadNum);
			Systems[threadNum].printX();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; P\n",threadNum);
			Systems[threadNum].printP();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; LnLike: %f\n",threadNum,LnPosterior);
			printf("\n");
		#endif

		} else {
		LnPosterior = -kali::infiniteVal;
		}
	return LnPosterior;
	}

double kali::calcLnPosterior(double *walkerPos, void *func_args, double &LnPrior, double &LnLikelihood) {

	int threadNum = omp_get_thread_num();

	kali::LnLikeArgs *ptr2Args = reinterpret_cast<kali::LnLikeArgs*>(func_args);
	kali::LnLikeArgs Args = *ptr2Args;

	kali::LnLikeData *Data = Args.Data;
	kali::MBHBCARMA *Systems = Args.Systems;
	kali::LnLikeData *ptr2Data = Data;
	double LnPosterior = 0.0, old_dt = 0.0;

	if (Systems[threadNum].checkMBHBCARMAParams(walkerPos) == 1) {
		old_dt = Systems[threadNum].get_dt();
		Systems[threadNum].setMBHBCARMA(walkerPos);
		Systems[threadNum].solveMBHBCARMA();
		Systems[threadNum].resetState();
		LnPrior = Systems[threadNum].computeLnPrior(ptr2Data);

		#ifdef DEBUG_CALCLNPOSTERIOR
			printf("calcLnPosterior - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum <  Systems[threadNum].get_r() + Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
				printf("%+17.16e ", walkerPos[dimNum]);
				}
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; System good!\n",threadNum);
			printf("calcLnPosterior - threadNum: %d; dt: %e\n",threadNum, Systems[threadNum].get_dt());
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; A\n",threadNum);
			Systems[threadNum].printA();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; w\n",threadNum);
			Systems[threadNum].printw();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; expw\n",threadNum);
			Systems[threadNum].printexpw();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; vr\n",threadNum);
			Systems[threadNum].printvr();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; vrInv\n",threadNum);
			Systems[threadNum].printvrInv();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; B\n",threadNum);
			Systems[threadNum].printB();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; C\n",threadNum);
			Systems[threadNum].printC();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; F\n",threadNum);
			Systems[threadNum].printF();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; Q\n",threadNum);
			Systems[threadNum].printQ();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; Sigma\n",threadNum);
			Systems[threadNum].printSigma();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; X\n",threadNum);
			Systems[threadNum].printX();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; P\n",threadNum);
			Systems[threadNum].printP();
			printf("\n");
            printf("calcLnPosterior - threadNum: %d; LnPrior: %e\n",threadNum, LnPrior);
			printf("\n");
			fflush(0);
		#endif

        LnLikelihood = Systems[threadNum].computeLnLikelihood(ptr2Data);
		LnPosterior = LnLikelihood + LnPrior;

		Systems[threadNum].set_dt(old_dt);
		Systems[threadNum].solveMBHBCARMA();
		//Systems[threadNum].resetState();

		#ifdef DEBUG_CALCLNPOSTERIOR
			printf("calcLnPosterior - threadNum: %d; walkerPos: ", threadNum);
			for (int dimNum = 0; dimNum <  Systems[threadNum].get_r() + Systems[threadNum].get_p() + Systems[threadNum].get_q() + 1; dimNum++) {
				printf("%+17.16e ", walkerPos[dimNum]);
				}
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; X\n", threadNum);
			Systems[threadNum].printX();
			printf("\n");
			printf("calcLnPosterior - threadNum: %d; P\n", threadNum);
			Systems[threadNum].printP();
			printf("\n");
			printf("calcLnLike - threadNum: %d; LnPosterior: %f\n", threadNum, LnPosterior);
			printf("\n");
		#endif

		} else {
        LnPrior = -kali::infiniteVal;
        LnLikelihood = -kali::infiniteVal;;
        LnPosterior = -kali::infiniteVal;
		}
	return LnPosterior;
	}

void kali::zeroMatrix(int nRows, int nCols, int* mat) {
	for (int colNum = 0; colNum < nCols; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < nRows; ++rowNum) {
			mat[rowNum + nRows*colNum] = 0;
			}
		}
	}

void kali::zeroMatrix(int nRows, int nCols, double* mat) {
	for (int colNum = 0; colNum < nCols; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < nRows; ++rowNum) {
			mat[rowNum + nRows*colNum] = 0.0;
			}
		}
	}

void kali::zeroMatrix(int nRows, int nCols, complex<double>* mat) {
	for (int colNum = 0; colNum < nCols; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < nRows; ++rowNum) {
			mat[rowNum + nRows*colNum] = 0.0;
			}
		}
	}

void kali::viewMatrix(int nRows, int nCols, int* mat) {
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			printf("%+d ",mat[j*nCols + i]);
			}
		printf("\n");
		}
	}

void kali::viewMatrix(int nRows, int nCols, double* mat) {
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			printf("%+8.7e ",mat[j*nCols + i]);
			}
		printf("\n");
		}
	}

void kali::viewMatrix(int nRows, int nCols, vector<double> mat) {
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			printf("%+8.7e ",mat[j*nCols + i]);
			}
		printf("\n");
		}
	}

void kali::viewMatrix(int nRows, int nCols, complex<double>* mat) {
	for (int i = 0; i < nRows; i++) {
		for (int j = 0; j < nCols; j++) {
			printf("%+8.7e%+8.7ei ",mat[j*nCols + i].real(),mat[j*nCols + i].imag());
			}
		printf("\n");
		}
	}

double kali::dtime() {
	double tseconds = 0.0;
	struct timeval mytime;
	gettimeofday(&mytime,(struct timezone*)0);
	tseconds = (double)(mytime.tv_sec + mytime.tv_usec*1.0e-6);
	return( tseconds );
	}

void kali::kron(int m, int n, double* A, int p, int q, double* B, double* C) {
	int alpha = 0;
	int beta = 0;
	int mp = m*p;
	int nq = n*q;
	for (int i = 0; i < m; i++) {
		for (int j = 0; j < n; j++) {
			for (int k = 0; k < p; k++) {
				#pragma omp simd
				for (int l = 0; l < q; l++) {
					alpha = p*i + k;
					beta = q*j + l;
					C[alpha + beta*nq] = A[i + j*n]*B[k + l*q];
					}
				}
			}
		}
	}

void kali::getSigma(int numR, int numP, int numQ, double *Theta, double *SigmaOut) {

	int r = numR, p = numP, q = numQ, pSq = p*p, qSq = q*q;
	lapack_int YesNo;
	complex<double> alpha = kali::complexOne, beta = kali::complexZero;

	complex<double> *A = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	complex<double> *ACopy = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	complex<double> *AScratch = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	complex<double> *w = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	complex<double> *vr = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	complex<double> *vrInv = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	lapack_int *ilo = static_cast<lapack_int*>(_mm_malloc(1*sizeof(lapack_int),64));
	lapack_int *ihi = static_cast<lapack_int*>(_mm_malloc(1*sizeof(lapack_int),64));
	double *abnrm = static_cast<double*>(_mm_malloc(1*sizeof(double),64));
	lapack_int *ipiv = static_cast<lapack_int*>(_mm_malloc(p*sizeof(lapack_int),64));
	double *scale = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	double *rconde = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	double *rcondv = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	complex<double> *C = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	complex<double> *B = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	complex<double> *BScratch = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));

	ilo[0] = 0;
	ihi[0] = 0;
	abnrm[0] = 0.0;

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		B[colCtr] = kali::complexZero;
		BScratch[colCtr] = kali::complexZero;
		ipiv[colCtr] = 0;
		scale[colCtr] = 0.0;
		rconde[colCtr] = 0.0;
		rcondv[colCtr] = 0.0;
		w[colCtr] = kali::complexZero;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			A[rowCtr + colCtr*p] = kali::complexZero;
			ACopy[rowCtr + colCtr*p] = kali::complexZero;
			AScratch[rowCtr + colCtr*p] = kali::complexZero;
			vr[rowCtr + colCtr*p] = kali::complexZero;
			vrInv[rowCtr + colCtr*p] = kali::complexZero;
			C[rowCtr + colCtr*p] = kali::complexZero;
			}
		}

	A[0] = -1.0*kali::complexOne*Theta[r];
	#pragma omp simd
	for (int i = 1; i < p; ++i) {
		A[i] = -1.0*kali::complexOne*Theta[r + i];
		A[i*p + (i - 1)] = kali::complexOne;
		}

	cblas_zcopy(pSq, A, 1, ACopy, 1); // Copy A into ACopy so that we can keep a clean working version of it.

	YesNo = LAPACKE_zgeevx(LAPACK_COL_MAJOR, 'B', 'N', 'V', 'N', p, ACopy, p, w, vrInv, 1, vr, p, ilo, ihi, scale, abnrm, rconde, rcondv); // Compute w and vr

	YesNo = LAPACKE_zlacpy(LAPACK_COL_MAJOR, 'B', p, p, vr, p, vrInv, p); // Copy vr into vrInv

	YesNo = LAPACKE_zgetrf(LAPACK_COL_MAJOR, p, p, vrInv, p, ipiv); // Compute LU factorization of vrInv == vr

	YesNo = LAPACKE_zgetri(LAPACK_COL_MAJOR, p, vrInv, p, ipiv); // Compute vrInv = inverse of vr from LU decomposition

	#pragma omp simd
	for (int rowCtr = 0; rowCtr < q + 1; rowCtr++) {
		B[p - 1 - rowCtr] = kali::complexOne*Theta[r + p + rowCtr];
		}

	// Start computation of C

	cblas_zgemv(CblasColMajor, CblasNoTrans, p, p, &alpha, vrInv, p, B, 1, &beta, BScratch, 1); // BScratch = vrInv*B

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			ACopy[rowCtr + colCtr*p] = BScratch[rowCtr]*B[colCtr]; // ACopy = BScratch*trans(B) = vrInv*b*trans(B)
			}
		}

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, &alpha, ACopy, p, vrInv, p, &beta, AScratch, p); // AScratch = ACopy*trans(vrInv) = vrInv*b*trans(B)*trans(vrInv)

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			C[rowCtr + colCtr*p] = (AScratch[rowCtr + colCtr*p] + AScratch[colCtr + rowCtr*p])/2.0; // C = (AScratch + trans(AScratch))/2 to ensure symmetry!
			}
		}

	for (int colNum = 0; colNum < p; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < p; ++rowNum) {
			ACopy[rowNum + p*colNum] = C[rowNum + p*colNum]*( - kali::complexOne)*(kali::complexOne/(w[rowNum] + w[colNum])); // ACopy[i,j] = = (C[i,j]*( - 1/(lambda[i] + lambda[j]))
		}
	}

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, ACopy, p, &beta, AScratch, p); // AScratch = vr*ACopy
	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, &alpha, AScratch, p, vr, p, &beta, ACopy, p); // ACopy = AScratch*trans(vr)

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			SigmaOut[rowCtr + colCtr*p] = ACopy[rowCtr + colCtr*p].real(); // Sigma = ACopy
			}
		}

	if (ilo) {
		_mm_free(ilo);
		ilo = nullptr;
		}
	if (ihi) {
		_mm_free(ihi);
		ihi = nullptr;
		}
	if (abnrm) {
		_mm_free(abnrm);
		abnrm = nullptr;
		}
	if (ipiv) {
		_mm_free(ipiv);
		ipiv = nullptr;
		}
	if (scale) {
		_mm_free(scale);
		scale = nullptr;
		}
	if (rconde) {
		_mm_free(rconde);
		rconde = nullptr;
		}
	if (rcondv) {
		_mm_free(rcondv);
		rcondv = nullptr;
		}
	if (A) {
		_mm_free(A);
		A = nullptr;
		}
	if (ACopy) {
		_mm_free(ACopy);
		ACopy = nullptr;
		}
	if (AScratch) {
		_mm_free(AScratch);
		AScratch = nullptr;
		}
	if (w) {
		_mm_free(w);
		w = nullptr;
		}
	if (vr) {
		_mm_free(vr);
		vr = nullptr;
		}
	if (vrInv) {
		_mm_free(vrInv);
		vrInv = nullptr;
		}
	if (B) {
		_mm_free(B);
		B = nullptr;
		}
	if (BScratch) {
		_mm_free(BScratch);
		BScratch = nullptr;
		}
	if (C) {
		_mm_free(C);
		C = nullptr;
		}

	}

double kali::d2r(double degreeVal) {
	return degreeVal*(pi/180.0);
	}

double kali::r2d(double radianVal) {
	return radianVal*(180.0/pi);
	}

double kali::KeplerEqn(const vector<double> &x, vector<double> &grad, void *p2Data) {
	KeplersEqnData *ptr2Data = reinterpret_cast<KeplersEqnData*>(p2Data);
	KeplersEqnData Data = *ptr2Data;
	if (!grad.empty()) {
		grad[0] = 1.0 - Data.eccentricity*cos(x[0]);
		}
	double funcVal = fabs(x[0] - Data.eccentricity*sin(x[0]) - Data.M);
	#ifdef DEBUG_KEPLEREQN
		printf("In Kepler's equation; funcVal = %+4.3e\n",funcVal);
	#endif
	return funcVal;
	}

int kali::MBHBCARMA::r = 8;

kali::MBHBCARMA::MBHBCARMA() {
	/*! Object that holds data and methods for performing C-ARMA analysis. DLM objects hold pointers to blocks of data that are set as required based on the size of the C-ARMA model.*/
	#ifdef DEBUG_CTORDLM
	int threadNum = omp_get_thread_num();
	printf("DLM - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	allocated = 0;
	isStable = 1;
	isInvertible = 1;
	isNotRedundant = 1;
	hasUniqueEigenValues = 1;
	hasPosSigma = 1;
	p = 0;
	q = 0;
	pSq = 0;
	dt = 0.0;

	ilo = nullptr; // len 1
	ihi = nullptr; // len 1
	abnrm = nullptr; // len 1

	// Arrays used to compute expm(A dt)
	w = nullptr; // len p
	expw = nullptr; // len pSq
	CARMatrix = nullptr; // len pSq
	CMAMatrix = nullptr; // len qSq
	CARw = nullptr; // len p
	CMAw = nullptr; //len p
	scale = nullptr;
	vr = nullptr;
	vrInv = nullptr;
	rconde = nullptr;
	rcondv = nullptr;
	ipiv = nullptr;

	Theta = nullptr;
	A = nullptr;
	ACopy = nullptr;
	AScratch = nullptr;
	AScratch2 = nullptr;
	B = nullptr;
	BScratch = nullptr;
	C = nullptr;
	I = nullptr;
	F = nullptr;
	Sigma = nullptr;
	Q = nullptr;
	T = nullptr;
	H = nullptr;
	R = nullptr;
	K = nullptr;
	X = nullptr;
	P = nullptr;
	XMinus = nullptr;
	PMinus = nullptr;
	VScratch = nullptr;
	MScratch = nullptr;

    m1 = 0.0;
	m2 = 0.0;
	rS1 = 0.0;
	rS2 = 0.0;
	totalMass = 0.0;
	massRatio = NAN;
	reducedMass = 0.0;
	eccentricity = 0.0;
	eccentricityFactor = 1.0;
	a1 = 0.0;
	a2 = 0.0;
	rPeribothron1 = 0.0;
	rPeribothron2 = 0.0;
	rApobothron1 = 0.0;
	rApobothron2 = 0.0;
	rPeribothronTot = 0.0;
	rApobothronTot = 0.0;
	omega1 = 0.0;
	omega2 = pi;
	inclination = pi/2.0;
	tau = 0.0;
	alpha1 = -0.44;
	alpha2 = -0.44;
	period = 0.0;
	epoch = 0.0;
	M = 0.0;
	E = 0.0;
	r1 = 0.0; // current distance of m1 from COM
	r2 = 0.0; // current distance of m2 from COM
	nu = 0.0; // current true anomoly of m1
	if (nu < 0.0) {
		nu += 2.0*pi;
		}
	theta1 = nu + omega1;
	theta2 = nu + omega2;
	beta1 = 0.0;
	beta2 = 0.0;
	_radialBetaFactor1 = 0.0;
	_radialBetaFactor2 = 0.0;
	radialBeta1 = 0.0;
	radialBeta2 = 0.0;
	dF1 = 0.0;
	dF2 = 0.0;
	bF1 = 0.0;
	bF2 = 0.0;
	totalFlux = 1.0;

	#ifdef DEBUG_CTORDLM
	printf("DLM - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	}

kali::MBHBCARMA::~MBHBCARMA() {

	#ifdef DEBUG_DTORDLM
	int threadNum = omp_get_thread_num();
	printf("~DLM - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	allocated = 0;
	isStable = 1;
	isInvertible = 1;
	isNotRedundant = 1;
	hasUniqueEigenValues = 1;
	hasPosSigma = 1;
    mbhbIsGood = 1;
	p = 0;
	q = 0;
	pSq = 0;
	dt = 0.0;

	ilo = nullptr;
	ihi = nullptr;
	abnrm = nullptr;
	w = nullptr;
	expw = nullptr;
	CARMatrix = nullptr;
	CMAMatrix = nullptr;
	CARw = nullptr;
	CMAw = nullptr;
	scale = nullptr;
	vr = nullptr;
	vrInv = nullptr;
	rconde = nullptr;
	rcondv = nullptr;
	ipiv = nullptr;

	Theta = nullptr;
	A = nullptr;
	ACopy = nullptr;
	AScratch = nullptr;
	AScratch2 = nullptr;
	B = nullptr;
	BScratch = nullptr;
	C = nullptr;
	I = nullptr;
	F = nullptr;
	Sigma = nullptr;
	Q = nullptr;
	T = nullptr;
	H = nullptr;
	R = nullptr;
	K = nullptr;
	X = nullptr;
	P = nullptr;
	XMinus = nullptr;
	PMinus = nullptr;
	VScratch = nullptr;
	MScratch = nullptr;

	#ifdef DEBUG_DTORDLM
	printf("~DLM - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	}

void kali::MBHBCARMA::allocMBHBCARMA(int numP, int numQ) {

	#ifdef DEBUG_ALLOCATECARMA
	int threadNum = omp_get_thread_num();
	printf("allocDLM - threadNum: %d; Starting... Address of System: %p\n",threadNum,this);
	#endif

	if ((numQ >= numP) or (numQ < 0)) {
		printf("FATAL LOGIC ERROR: numP MUST BE > numQ >= 0!\n");
		exit(1);
		}
	p = numP;
	q = numQ;
	allocated = 0;
	pSq = p*p;
	qSq = q*q;

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating ilo, ihi, abnrm, ipiv, scale, rconde, rcondv Address of System: %p\n",threadNum,this);
	#endif

	ilo = static_cast<lapack_int*>(_mm_malloc(1*sizeof(lapack_int),64));
	ihi = static_cast<lapack_int*>(_mm_malloc(1*sizeof(lapack_int),64));
	allocated += 2*sizeof(lapack_int);

	ilo[0] = 0;
	ihi[0] = 0;

	abnrm = static_cast<double*>(_mm_malloc(1*sizeof(double),64));
	allocated += sizeof(double);

	abnrm[0] = 0.0;

	ipiv = static_cast<lapack_int*>(_mm_malloc(p*sizeof(lapack_int),64));
	allocated += p*sizeof(lapack_int);

	scale = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	rconde = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	rcondv = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	allocated += 3*p*sizeof(double);

	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		ipiv[rowCtr] = 0;
		scale[rowCtr] = 0.0;
		rconde[rowCtr] = 0.0;
		rcondv[rowCtr] = 0.0;
		}

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating w, CARw, B, BScratch Address of System: %p\n",threadNum,this);
	#endif

	w = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	CARw = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	B = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	BScratch = static_cast<complex<double>*>(_mm_malloc(p*sizeof(complex<double>),64));
	allocated += 4*p*sizeof(complex<double>);

	if (q > 0) {

		#ifdef DEBUG_ALLOCATECARMA
		printf("allocDLM - threadNum: %d; Allocating CMAw, CMAMatrix Address of System: %p\n",threadNum,this);
		#endif

		CMAw = static_cast<complex<double>*>(_mm_malloc(q*sizeof(complex<double>),64));
		CMAMatrix = static_cast<complex<double>*>(_mm_malloc(qSq*sizeof(complex<double>),64));
		allocated += q*sizeof(complex<double>);
		allocated += qSq*sizeof(complex<double>);

		for (int colCtr = 0; colCtr < q; ++colCtr) {
			CMAw[colCtr] = kali::complexZero;
			#pragma omp simd
			for (int rowCtr = 0; rowCtr < q; ++rowCtr) {
				CMAMatrix[rowCtr + colCtr*q] = kali::complexZero;
				}
			}
		}

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating CARMatrix, expw, vr, vrInv, A, ACopy, AScratch, AScratch2 Address of System: %p\n",threadNum,this);
	#endif

	CARMatrix = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	expw = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	vr = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	vrInv = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	A = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	C = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	ACopy = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	AScratch = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	AScratch2 = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>),64));
	allocated += 8*pSq*sizeof(complex<double>);

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		w[colCtr] = kali::complexZero;
		CARw[colCtr] = kali::complexZero;
		B[colCtr] = 0.0;
		BScratch[colCtr] = 0.0;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			CARMatrix[rowCtr + colCtr*p] = kali::complexZero;
			expw[rowCtr + colCtr*p] = kali::complexZero;
			vr[rowCtr + colCtr*p] = kali::complexZero;
			vrInv[rowCtr + colCtr*p] = kali::complexZero;
			A[rowCtr + colCtr*p] = kali::complexZero;
			C[rowCtr + colCtr*p] = kali::complexZero;
			ACopy[rowCtr + colCtr*p] = kali::complexZero;
			AScratch[rowCtr + colCtr*p] = kali::complexZero;
			AScratch2[rowCtr + colCtr*p] = kali::complexZero;
			}
		}

	Theta = static_cast<double*>(_mm_malloc((kali::MBHBCARMA::r + p + q + 1)*sizeof(double),64));
	allocated += (kali::MBHBCARMA::r+p+q+1)*sizeof(double);

	for (int rowCtr = 0; rowCtr < kali::MBHBCARMA::r + p + q + 1; ++rowCtr) {
		Theta[rowCtr] = 0.0;
		}

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating H, K, X, XMinus, VScratch, I, F, Sigma, Q, T, P, PMinus, MScratch Address of System: %p\n",threadNum,this);
	#endif

	H = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	K = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	X = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	XMinus = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	VScratch = static_cast<double*>(_mm_malloc(p*sizeof(double),64));
	//allocated += 6*p*sizeof(double);
	allocated += 5*p*sizeof(double);

	I = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	F = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	Sigma = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	Q = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	T = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	P = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	PMinus = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	MScratch = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	allocated += 7*pSq*sizeof(double);

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		H[colCtr] = 0.0;
		K[colCtr] = 0.0;
		X[colCtr] = 0.0;
		XMinus[colCtr] = 0.0;
		VScratch[colCtr] = 0.0;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			I[rowCtr + colCtr*p] = 0.0;
			F[rowCtr + colCtr*p] = 0.0;
			Sigma[rowCtr + colCtr*p] = 0.0;
			Q[rowCtr + colCtr*p] = 0.0;
			T[rowCtr + colCtr*p] = 0.0;
			P[rowCtr + colCtr*p] = 0.0;
			PMinus[rowCtr + colCtr*p] = 0.0;
			MScratch[rowCtr + colCtr*p] = 0.0;
			}
		}

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Allocating R Address of System: %p\n",threadNum,this);
	#endif

	R = static_cast<double*>(_mm_malloc(sizeof(double),64));
	allocated += sizeof(double);

	R[0] = 0.0;

	#pragma omp simd
	for (int i = 1; i < p; ++i) {
		A[i*p + (i - 1)] = kali::complexOne;
		I[(i - 1)*p + (i - 1)] = 1.0;
		}
	I[(p - 1)*p + (p - 1)] = 1.0;

	#ifdef DEBUG_ALLOCATECARMA
	printf("allocDLM - threadNum: %d; Finishing... Address of System: %p\n",threadNum,this);
	#endif

	}

void kali::MBHBCARMA::deallocMBHBCARMA() {

	#ifdef DEBUG_DEALLOCATECARMA
	int threadNum = omp_get_thread_num();
	printf("deallocDLM - threadNum: %d; Starting... Address of System: %p\n",threadNum,this);
	#endif

	if (ilo) {
		_mm_free(ilo);
		ilo = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated ilo Address of System: %p\n",threadNum,this);
	#endif

	if (ihi) {
		_mm_free(ihi);
		ihi = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated ihi Address of System: %p\n",threadNum,this);
	#endif

	if (abnrm) {
		_mm_free(abnrm);
		abnrm = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated abnrm Address of System: %p\n",threadNum,this);
	#endif

	if (ipiv) {
		_mm_free(ipiv);
		ipiv = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated ipiv Address of System: %p\n",threadNum,this);
	#endif

	if (scale) {
		_mm_free(scale);
		scale = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated scale Address of System: %p\n",threadNum,this);
	#endif

	if (rconde) {
		_mm_free(rconde);
		rconde = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated rconde Address of System: %p\n",threadNum,this);
	#endif

	if (rcondv) {
		_mm_free(rcondv);
		rcondv = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated rcondv Address of System: %p\n",threadNum,this);
	#endif

	if (w) {
		_mm_free(w);
		w = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated w Address of System: %p\n",threadNum,this);
	#endif

	if (CARw) {
		_mm_free(CARw);
		CARw = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated CARw Address of System: %p\n",threadNum,this);
	#endif

	if (B) {
		_mm_free(B);
		B = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated B Address of System: %p\n",threadNum,this);
	#endif

	if (BScratch) {
		_mm_free(BScratch);
		BScratch = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated BScratch Address of System: %p\n",threadNum,this);
	#endif

	if (CMAw) {
		_mm_free(CMAw);
		CMAw = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated CMAw Address of System: %p\n",threadNum,this);
	#endif

	if (CMAMatrix) {
		_mm_free(CMAMatrix);
		CMAMatrix = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated CMAMatrix Address of System: %p\n",threadNum,this);
	#endif

	if (CARMatrix) {
		_mm_free(CARMatrix);
		CARMatrix = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated CARMatrix Address of System: %p\n",threadNum,this);
	#endif

	if (expw) {
		_mm_free(expw);
		expw = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated expw Address of System: %p\n",threadNum,this);
	#endif

	if (vr) {
		_mm_free(vr);
		vr = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated vr Address of System: %p\n",threadNum,this);
	#endif

	if (vrInv) {
		_mm_free(vrInv);
		vrInv = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated vrInv Address of System: %p\n",threadNum,this);
	#endif

	if (A) {
		_mm_free(A);
		A = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated A Address of System: %p\n",threadNum,this);
	#endif

	if (ACopy) {
		_mm_free(ACopy);
		ACopy = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated ACopy Address of System: %p\n",threadNum,this);
	#endif

	if (AScratch) {
		_mm_free(AScratch);
		AScratch = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated AScratch Address of System: %p\n",threadNum,this);
	#endif

	if (AScratch2) {
		_mm_free(AScratch2);
		AScratch2 = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated AScratch2 Address of System: %p\n",threadNum,this);
	#endif

	if (C) {
		_mm_free(C);
		C = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated C Address of System: %p\n",threadNum,this);
	#endif

	if (Theta) {
		_mm_free(Theta);
		Theta = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated Theta Address of System: %p\n",threadNum,this);
	#endif

	if (H) {
		_mm_free(H);
		H = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated H Address of System: %p\n",threadNum,this);
	#endif

	if (K) {
		_mm_free(K);
		K = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated K Address of System: %p\n",threadNum,this);
	#endif

	if (X) {
		_mm_free(X);
		X = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated X Address of System: %p\n",threadNum,this);
	#endif

	if (XMinus) {
		_mm_free(XMinus);
		XMinus = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated XMinus Address of System: %p\n",threadNum,this);
	#endif

	if (VScratch) {
		_mm_free(VScratch);
		VScratch = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated VScratch Address of System: %p\n",threadNum,this);
	#endif

	if (I) {
		_mm_free(I);
		I = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated I Address of System: %p\n",threadNum,this);
	#endif

	if (F) {
		_mm_free(F);
		F = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated F Address of System: %p\n",threadNum,this);
	#endif

	if (Sigma) {
		_mm_free(Sigma);
		Sigma = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated Sigma Address of System: %p\n",threadNum,this);
	#endif

	if (Q) {
		_mm_free(Q);
		Q = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated Q Address of System: %p\n",threadNum,this);
	#endif

	if (T) {
		_mm_free(T);
		T = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated T Address of System: %p\n",threadNum,this);
	#endif

	if (P) {
		_mm_free(P);
		P = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated P Address of System: %p\n",threadNum,this);
	#endif

	if (PMinus) {
		_mm_free(PMinus);
		PMinus = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated PMinus Address of System: %p\n",threadNum,this);
	#endif

	if (MScratch) {
		_mm_free(MScratch);
		MScratch = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated MScratch Address of System: %p\n",threadNum,this);
	#endif

	if (R) {
		_mm_free(R);
		R = nullptr;
		}

	#ifdef DEBUG_DEALLOCATECARMA_DEEP
	printf("deallocDLM - threadNum: %d; Deallocated R Address of System: %p\n",threadNum,this);
	#endif

	#ifdef DEBUG_DEALLOCATECARMA
	printf("deallocDLM - threadNum: %d; Finishing... Address of System: %p\n",threadNum,this);
	#endif
	}

int kali::MBHBCARMA::get_r() {
	return kali::MBHBCARMA::r;
	}

int kali::MBHBCARMA::get_p() {
	return p;
	}

int kali::MBHBCARMA::get_q() {
	return q;
	}

double kali::MBHBCARMA::get_dt() {
	return dt;
	}

void kali::MBHBCARMA::set_dt(double new_dt) {
	dt = new_dt;
	}

int kali::MBHBCARMA::get_allocated() {
	return allocated;
	}

void kali::MBHBCARMA::getCARRoots(complex<double>*& CARRoots) {
	CARRoots = CARw;
	}

void kali::MBHBCARMA::getCMARoots(complex<double>*& CMARoots) {
	CMARoots = CMAw;
	}

void kali::MBHBCARMA::printX() {
	viewMatrix(p,1,X);
	}

void kali::MBHBCARMA::getX(double *newX) {
	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		newX[rowCtr] = X[rowCtr];
		}
	}

void kali::MBHBCARMA::setX(double *newX) {
	#pragma omp simd
	for (int i = 0; i < p; ++i) {
		X[i] = newX[i];
		}
	}

void kali::MBHBCARMA::printP() {
	viewMatrix(p,p,P);
	}

void kali::MBHBCARMA::getP(double *newP) {
	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			newP[rowCtr + p*colCtr] = P[rowCtr + p*colCtr];
			}
		}
	}

void kali::MBHBCARMA::setP(double* newP) {
	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			P[rowCtr + p*colCtr] = newP[rowCtr + p*colCtr];
			}
		}
	}


void kali::MBHBCARMA::printdt() {
	printf("dt: %e\n", dt);
	}

void kali::MBHBCARMA::printA() {
	viewMatrix(p,p,A);
	}

const complex<double>* kali::MBHBCARMA::getA() const {
	return A;
	}

void kali::MBHBCARMA::printvr() {
	viewMatrix(p,p,vr);
	}

const complex<double>* kali::MBHBCARMA::getvr() const {
	return vr;
	}

void kali::MBHBCARMA::printvrInv() {
	viewMatrix(p,p,vrInv);
	}

const complex<double>* kali::MBHBCARMA::getvrInv() const {
	return vrInv;
	}

void kali::MBHBCARMA::printw() {
	viewMatrix(p,1,w);
	}

const complex<double>* kali::MBHBCARMA::getw() const {
	return w;
	}

void kali::MBHBCARMA::printexpw() {
	viewMatrix(p,p,expw);
	}

const complex<double>* kali::MBHBCARMA::getexpw() const {
	return expw;
	}

void kali::MBHBCARMA::printB() {
	viewMatrix(p,1,B);
	}

const complex<double>* kali::MBHBCARMA::getB() const {
	return B;
	}

void kali::MBHBCARMA::printC() {
	viewMatrix(p,p,C);
	}

const complex<double>* kali::MBHBCARMA::getC() const {
	return C;
	}

void kali::MBHBCARMA::printF() {
	viewMatrix(p,p,F);
	}

const double* kali::MBHBCARMA::getF() const {
	return F;
	}

void kali::MBHBCARMA::printQ() {
	viewMatrix(p,p,Q);
	}

const double* kali::MBHBCARMA::getQ() const {
	return Q;
	}

void kali::MBHBCARMA::printT() {
	viewMatrix(p,p,T);
	}

const double* kali::MBHBCARMA::getT() const {
	return T;
	}

void kali::MBHBCARMA::printSigma() {
	viewMatrix(p,p,Sigma);
	}

const double* kali::MBHBCARMA::getSigma() const {
	return Sigma;
	}

/*!
 * Checks the validity of the supplied C-ARMA parameters.
 * @param[in]  Theta  \f$\Theta\f$ contains \f$p\f$ CAR parameters followed by \f$q+1\f$ CMA parameters, i.e. \f$\Theta = [a_{1}, a_{2}, ..., a_{p-1}, a_{p}, b_{0}, b_{1}, ..., b_{q-1}, b_{q}]\f$, where we follow the notation in Brockwell 2001, Handbook of Statistics, Vol 19
 */
int kali::MBHBCARMA::checkMBHBCARMAParams(double *ThetaIn /**< [in]  */) {
	/*!< \brief Function to check the validity of the MBHBCARMA parameters.


	*/

	#ifdef DEBUG_CHECKMBHBCARMAPARAMS
	int threadNum = omp_get_thread_num();
	printf("checkMBHBCARMAParams - threadNum: %d; Address of System: %p\n",threadNum,this);
	#endif

	isStable = 1;
	isInvertible = 1;
	isNotRedundant = 1;
	hasUniqueEigenValues = 1;
	hasPosSigma = 1;
    mbhbIsGood = 1;

	double a1Val = kali::Parsec*ThetaIn[0];
	double a2Val = kali::Parsec*ThetaIn[1];

    if (a1Val <= 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("a1: %4.3e\n",a1Val);
			printf("a1LLim: %4.3e\n",0.0);
		#endif
		}

    if (a2Val <= 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("a2: %4.3e\n",a2Val);
			printf("a2LLim: %4.3e\n",0.0);
		#endif
		}

	if (a2Val < a1Val) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("a2: %4.3e\n",a2Val);
			printf("a2LLim: %4.3e\n",a1Val);
		#endif
		}

	double periodVal = kali::Day*ThetaIn[2];

    if (periodVal <= 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("period: %4.3e\n",periodVal);
			printf("periodLLim: %4.3e\n",0.0);
		#endif
		}

	double totalMassVal = (kali::fourPiSq*pow(a1Val + a2Val, 3.0))/(kali::G*pow(periodVal, 2.0));
	double massRatioVal = a1Val/a2Val;
	double m1Val = totalMassVal*(1.0/(1.0 + massRatioVal));
	double m2Val = totalMassVal*(massRatioVal/(1.0 + massRatioVal));

    if (m1Val < 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBPARAMS
			printf("m1: %4.3e\n",m1Val);
			printf("m1LLim: %4.3e\n",0.0);
		#endif
		}

    if (m2Val < 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBPARAMS
			printf("m2: %4.3e\n",m2Val);
			printf("m2LLim: %4.3e\n",0.0);
		#endif
		}

	double rS1Val = (2.0*kali::G*m1Val)/(pow(kali::c, 2.0));
	double rS2Val = (2.0*kali::G*m2Val)/(pow(kali::c, 2.0));

	double eccentricityVal = ThetaIn[3];
	if (eccentricityVal < 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("eccentricity: %4.3e\n",eccentricityVal);
			printf("eccentricityLLim: %4.3e\n",0.0);
		#endif
		}
	if (eccentricityVal >= 1.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("eccentricity: %4.3e\n",eccentricityVal);
			printf("eccentricityULim: %4.3e\n",1.0);
		#endif
		}

	double rPeribothronTotVal = a1Val*(1.0 - eccentricityVal) + a2Val*(1.0 - eccentricityVal);
	if (rPeribothronTotVal <= 10.0*(rS1Val + rS2Val)) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("rPeribothronTot: %+4.3e\n",rPeribothronTotVal);
			printf("rPeribothronTotLLim: %+4.3e\n", 10.0*(rS1Val + rS2Val));
		#endif
		}

	double omega1Val = d2r(ThetaIn[4]);
	if (omega1Val < 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("omega1: %4.3e\n",omega1Val);
			printf("omega1LLim: %4.3e\n",0.0);
		#endif
		}
	if (omega1Val >= kali::twoPi) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("omega1: %4.3e\n",omega1Val);
			printf("omega1ULim: %4.3e\n",kali::twoPi);
		#endif
		}

	double inclinationVal = d2r(ThetaIn[5]);
	if (inclinationVal < 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("inclination: %4.3e\n",inclinationVal);
			printf("inclinationLLim: %4.3e\n",0.0);
		#endif
		}
	if (inclinationVal > kali::halfPi) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("inclination: %4.3e\n",inclinationVal);
			printf("inclinationULim: %4.3e\n",kali::halfPi);
		#endif
		}

	/*double tauVal = ThetaIn[6]*kali::Day;
    if (tauVal < 0.0) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("tau: %4.3e\n",tauVal);
			printf("tauLLim: %4.3e\n",0.0);
		#endif
		}
    if (tauVal > periodVal) {
		mbhbIsGood = 0;
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("tau: %4.3e\n",tauVal);
			printf("tauULim: %4.3e\n",periodVal);
		#endif
    }*/

	double totalFluxVal = ThetaIn[7];
	if (totalFluxVal <= 0.0) {
		#ifdef DEBUG_CHECKMBHBCARMAPARAMS
			printf("totalFlux: %4.3e\n",totalFluxVal);
			printf("totalFluxLLim: %4.3e\n",0.0);
		#endif
		mbhbIsGood = 0;
		}

	if (ThetaIn[kali::MBHBCARMA::r + p] <= 0.0) {
		hasPosSigma = 0;
		}

	for (int rowCtr = 0; rowCtr < p; rowCtr++) {
		#pragma omp simd
		for (int colCtr = 0; colCtr < p; colCtr++) {
			CARMatrix[rowCtr + p*colCtr] = kali::complexZero; // Reset matrix.
			}
		}

	CARMatrix[p*(p-1)] = -1.0*kali::complexOne*ThetaIn[kali::MBHBCARMA::r + p - 1]; // The first row has no 1s so we just set the rightmost entry equal to -alpha_p
	#pragma omp simd
	for (int rowCtr = 1; rowCtr < p; rowCtr++) {
		CARMatrix[rowCtr+(p-1)*p] = -1.0*kali::complexOne*ThetaIn[kali::MBHBCARMA::r + p - 1 - rowCtr]; // Rightmost column of CARMatrix equals -alpha_k where 1 < k < p.
		CARMatrix[rowCtr+(rowCtr-1)*p] = kali::complexOne; // CARMatrix has Identity matrix in bottom left.
		}
	ilo[0] = 0;
	ihi[0] = 0;
	abnrm[0] = 0.0;
	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		CARw[rowCtr] = kali::complexZero;
		scale[rowCtr] = 0.0;
		rconde[rowCtr] = 0.0;
		rcondv[rowCtr] = 0.0;
		}
	#ifdef DEBUG_CHECKARMAPARAMS
	printf("checkMBHBCARMAParams - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%f ",Theta[dimNum]);
		}
	printf("\n");
	printf("checkMBHBCARMAParams - threadNum: %d; CARMatrix\n",threadNum);
	viewMatrix(p,p,CARMatrix);
	#endif

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	lapack_int YesNo;
	//YesNo = LAPACKE_zgeevx(LAPACK_COL_MAJOR, 'B', 'N', 'N', 'N', p, CARMatrix, p, CARw, vrInv, p, vr, p, ilo, ihi, scale, abnrm, rconde, rcondv); // NOT WORKING!!!
	YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'N', p, CARMatrix, p, CARw, vrInv, p, vr, p);
	//YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'N', p, CARMatrix, p, CARw, vrInv, p, vr, p);

	for (int i = 0; i < p; i++) {

		#ifdef DEBUG_CHECKARMAPARAMS
		printf("checkMBHBCARMAParams - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%f ",Theta[dimNum]);
			}
		printf("\n");
		printf("checkMBHBCARMAParams - threadNum: %d; Root: %+f%+fi; Len: %f\n",threadNum, CARw[i].real(), CARw[i].imag(), abs(CARw[i]));
		#endif

		if (CARw[i].real() >= 0.0) {

			#ifdef DEBUG_CHECKARMAPARAMS
			printf("checkMBHBCARMAParams - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
				printf("%f ",Theta[dimNum]);
				}
			printf("\n");
			printf("checkMBHBCARMAParams - threadNum: %d; badRoot!!!: %+f%+fi; Len: %f\n",threadNum,CARw[i].real(), CARw[i].imag(),abs(CARw[i]));
			#endif

			isStable = 0;
			}

		for (int j = i+1; j < p; j++) { // Only need to check e-values against each other once.
			if (CARw[i] == CARw[j]) {
				hasUniqueEigenValues = 0;
				}
			}
		}

	if (q > 0) {
		for (int rowCtr = 0; rowCtr < q; ++rowCtr) {
			#pragma omp simd
			for (int colCtr = 0; colCtr < q; colCtr++) {
				CMAMatrix[rowCtr + q*colCtr] = kali::complexZero; // Initialize matrix.
				}
			}
		CMAMatrix[(q-1)*q] = -1.0*kali::complexOne*ThetaIn[kali::MBHBCARMA::r + p]/ThetaIn[kali::MBHBCARMA::r + p + q]; // MAMatrix has -beta_q/-beta_0 at top right!
		#pragma omp simd
		for (int rowCtr = 1; rowCtr < q; ++rowCtr) {
			CMAMatrix[rowCtr + (q - 1)*q] = -1.0*kali::complexOne*ThetaIn[kali::MBHBCARMA::r + p + rowCtr]/ThetaIn[kali::MBHBCARMA::r + p + q]; // Rightmost column of MAMatrix has -MA coeffs.
			CMAMatrix[rowCtr + (rowCtr - 1)*q] = kali::complexOne; // MAMatrix has Identity matrix in bottom left.
			}
		ilo[0] = 0;
		ihi[0] = 0;
		abnrm[0] = 0.0;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < q; ++rowCtr) {
			CMAw[rowCtr] = kali::complexZero;
			}
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			scale[rowCtr] = 0.0;
			rconde[rowCtr] = 0.0;
			rcondv[rowCtr] = 0.0;
			}
		#ifdef DEBUG_CHECKARMAPARAMS
		printf("checkMBHBCARMAParams - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%f ",Theta[dimNum]);
			}
		printf("\n");
		printf("checkMBHBCARMAParams - threadNum: %d; CMAMatrix\n",threadNum);
		viewMatrix(q,q,CMAMatrix);
		#endif

		//YesNo = LAPACKE_zgeevx(LAPACK_COL_MAJOR, 'B', 'N', 'V', 'N', q, CMAMatrix, q, CMAw, nullptr, 1, vr, q, ilo, ihi, scale, abnrm, rconde, rcondv); // NOT WORKING!!!!
		YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'N', q, CMAMatrix, q, CMAw, vrInv, q, vr, q);
		//YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'N', q, CMAMatrix, q, CMAw, vrInv, p, vr, p);

		for (int i = 0; i < q; i++) {

			#ifdef DEBUG_CHECKARMAPARAMS
			printf("checkMBHBCARMAParams - threadNum: %d; walkerPos: ",threadNum);
			for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
				printf("%f ",Theta[dimNum]);
				}
			printf("\n");
			printf("checkMBHBCARMAParams - threadNum: %d; Root: %+f%+fi; Len: %f\n",threadNum, CMAw[i].real(), CMAw[i].imag(), abs(CMAw[i]));
			#endif

			if (CMAw[i].real() > 0.0) {

				#ifdef DEBUG_CHECKARMAPARAMS
				printf("checkMBHBCARMAParams - threadNum: %d; walkerPos: ",threadNum);
				for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
					printf("%f ",Theta[dimNum]);
					}
				printf("\n");
				printf("checkMBHBCARMAParams - threadNum: %d; badRoot!!!: %+f%+fi; Len: %f\n",threadNum,CMAw[i].real(), CMAw[i].imag(),abs(CMAw[i]));
				#endif

				isInvertible = 0;
				}
			}

		for (int i = 1; i < p; i++) {
			for (int j = 1; j < q; j++) {
				if (CARw[i] == CMAw[j]) {
					isNotRedundant = 0;
					}
				}
			}
		} else if (q == 0) {
		if (Theta[kali::MBHBCARMA::r+p] < 0) {
			isInvertible = 0;
			}
		} else {
		printf("FATAL LOGIC ERROR: numP MUST BE > numQ >= 0!\n");
		exit(1);
		}

	#ifdef DEBUG_CHECKARMAPARAMS
	printf("checkMBHBCARMAParams - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%f ",Theta[dimNum]);
		}
	printf("\n");
	printf("checkMBHBCARMAParams - threadNum: %d; isStable: %d\n",threadNum,isStable);
	printf("checkMBHBCARMAParams - threadNum: %d; isInvertible: %d\n",threadNum,isInvertible);
	printf("checkMBHBCARMAParams - threadNum: %d; isNotRedundant: %d\n",threadNum,isNotRedundant);
	printf("checkMBHBCARMAParams - threadNum: %d; hasUniqueEigenValues: %d\n",threadNum,hasUniqueEigenValues);
	printf("checkMBHBCARMAParams - threadNum: %d; hasPosSigma: %d\n",threadNum,hasPosSigma);
    printf("checkMBHBCARMAParams - threadNum: %d; mbhbIsGood: %d\n",threadNum,mbhbIsGood);
	printf("\n");
	printf("\n");
	#endif

	return isStable*isInvertible*isNotRedundant*hasUniqueEigenValues*hasPosSigma*mbhbIsGood;
} // checkMBHBCARMAPArams

void kali::MBHBCARMA::setMBHBCARMA(double *ThetaIn) {

	complex<double> alpha = kali::complexOne, beta = kali::complexZero;

	#if (defined DEBUG_SETMBHBCARMA) || (defined DEBUG_SETMBHBCARMA_C)
	int threadNum = omp_get_thread_num();
	#endif

	#ifdef DEBUG_SETMBHBCARMA
	printf("setMBHBCARMA - threadNum: %d; Address of System: %p\n",threadNum,this);
	printf("\n");
	#endif

	#pragma omp simd
	for (int rowCtr = 0; rowCtr < kali::MBHBCARMA::r + p + q + 1; ++rowCtr) {
		Theta[rowCtr] = ThetaIn[rowCtr];
		}

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			A[rowCtr + colCtr*p] = kali::complexZero;
			//A[rowCtr + colCtr*p].imag(0.0);
			}
		}

	A[0] = -1.0*kali::complexOne*Theta[kali::MBHBCARMA::r];
	#pragma omp simd
	for (int i = 1; i < p; ++i) {
		A[i] = -1.0*kali::complexOne*Theta[kali::MBHBCARMA::r + i];
		A[i*p + (i - 1)] = kali::complexOne;
		}

	cblas_zcopy(pSq, A, 1, ACopy, 1); // Copy A into ACopy so that we can keep a clean working version of it.

	#ifdef DEBUG_SETMBHBCARMA
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; A\n",threadNum);
	viewMatrix(p,p,A);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; ACopy\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif

	ilo[0] = 0;
	ihi[0] = 0;
	abnrm[0] = 0.0;
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		w[rowCtr] = kali::complexZero;
		scale[rowCtr] = 0.0;
		rconde[rowCtr] = 0.0;
		rcondv[rowCtr] = 0.0;
		#pragma omp simd
		for (int colCtr = 0; colCtr < p; ++colCtr) {
			vr[rowCtr + colCtr*p] = kali::complexZero;
			vrInv[rowCtr + colCtr*p] = kali::complexZero;
			}
		}

	lapack_int YesNo;
	YesNo = LAPACKE_zgeevx(LAPACK_COL_MAJOR, 'B', 'N', 'V', 'N', p, ACopy, p, w, vrInv, 1, vr, p, ilo, ihi, scale, abnrm, rconde, rcondv); // Compute w and vr
	//YesNo = LAPACKE_zgeev(LAPACK_COL_MAJOR, 'N', 'V', p, ACopy, p, w, vrInv, p, vr, p);

	YesNo = LAPACKE_zlacpy(LAPACK_COL_MAJOR, 'B', p, p, vr, p, vrInv, p); // Copy vr into vrInv

	YesNo = LAPACKE_zgetrf(LAPACK_COL_MAJOR, p, p, vrInv, p, ipiv); // Compute LU factorization of vrInv == vr

	YesNo = LAPACKE_zgetri(LAPACK_COL_MAJOR, p, vrInv, p, ipiv); // Compute vrInv = inverse of vr from LU decomposition

	#ifdef DEBUG_SETMBHBCARMA
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[+ dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; w\n",threadNum);
	viewMatrix(p,1,w);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; vr\n",threadNum);
	viewMatrix(p,p,vr);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; vrInv\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	#endif

	#pragma omp simd
	for (int rowCtr = 0; rowCtr < q + 1; rowCtr++) {
		B[p - 1 - rowCtr] = kali::complexOne*Theta[kali::MBHBCARMA::r + p + rowCtr];
		}

	#ifdef DEBUG_SETMBHBCARMA
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; B\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	#endif

	// Start computation of C

	#ifdef DEBUG_SETMBHBCARMA_C
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; vrInv (Before)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; B (Before)\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; BScratch = vrInv*B (Before)\n",threadNum);
	viewMatrix(p,1,BScratch);
	printf("\n");
	#endif

	cblas_zgemv(CblasColMajor, CblasNoTrans, p, p, &alpha, vrInv, p, B, 1, &beta, BScratch, 1); // BScratch = vrInv*B

	#ifdef DEBUG_SETMBHBCARMA_C
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; vrInv (After)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; B (After)\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; BScratch = vrInv*B (After)\n",threadNum);
	viewMatrix(p,1,BScratch);
	printf("\n");
	#endif

	#ifdef DEBUG_SETMBHBCARMA_C
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; B (Before)\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; BScratch (Before)\n",threadNum);
	viewMatrix(p,1,BScratch);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; ACopy = BScratch*B (Before)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			ACopy[rowCtr + colCtr*p] = BScratch[rowCtr]*B[colCtr]; // ACopy = BScratch*trans(B) = vrInv*b*trans(B)
			}
		}

	#ifdef DEBUG_SETMBHBCARMA_C
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; B (After)\n",threadNum);
	viewMatrix(p,1,B);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; BScratch (After)\n",threadNum);
	viewMatrix(p,1,BScratch);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; ACopy = BScratch*B (After)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif


	#ifdef DEBUG_SETMBHBCARMA_C
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; ACopy (Before)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; vrInv (Before)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; AScratch = ACopy*trans(vrInv) (Before)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	#endif

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, &alpha, ACopy, p, vrInv, p, &beta, AScratch, p); // AScratch = ACopy*trans(vrInv) = vrInv*b*trans(B)*trans(vrInv)

	#ifdef DEBUG_SETMBHBCARMA_C
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; ACopy (After)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; vrInv (After)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; AScratch = ACopy*trans(vrInv) (After)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	#endif

	#ifdef DEBUG_SETMBHBCARMA_C
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; AScratch (Before)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; C = AScratch (Before)\n",threadNum);
	viewMatrix(p,p,C);
	printf("\n");
	#endif

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			C[rowCtr + colCtr*p] = (AScratch[rowCtr + colCtr*p] + AScratch[colCtr + rowCtr*p])/2.0; // C = (AScratch + trans(AScratch))/2 to ensure symmetry!
			}
		}

	#ifdef DEBUG_SETMBHBCARMA_C
	printf("setMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+7.6e ",Theta[dimNum]);
		}
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; AScratch (After)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	printf("setMBHBCARMA - threadNum: %d; C = AScratch (After)\n",threadNum);
	viewMatrix(p,p,C);
	printf("\n");
	#endif

	H[0] = 1.0;

    a1 = Theta[0]*kali::Parsec;
	a2 = Theta[1]*kali::Parsec;
	period = Theta[2]*kali::Day;
	totalMass = (kali::fourPiSq*pow(a1 + a2, 3.0))/(kali::G*pow(period, 2.0));
	massRatio = a1/a2;
	m1 = totalMass*(1.0/(1.0 + massRatio));
	m2 = totalMass*(massRatio/(1.0 + massRatio));
	rS1 = (2.0*kali::G*m1)/(pow(kali::c, 2.0));
	rS2 = (2.0*kali::G*m2)/(pow(kali::c, 2.0));
	reducedMass = m1*m2/(m1 + m2);
	eccentricity = Theta[3];
	eccentricityFactor = sqrt((1.0 + eccentricity)/(1.0 - eccentricity));
	rPeribothron1 = a1*(1.0 - eccentricity);
	rPeribothron2 = a2*(1.0 - eccentricity);
	rPeribothronTot = rPeribothron1 + rPeribothron2;
	rApobothron1 = a1*(1.0 + eccentricity);
	rApobothron2 = a2*(1.0 + eccentricity);
	rApobothronTot = rApobothron1 + rApobothron2;
	omega1 = d2r(Theta[4]);
	omega2 = omega1 + kali::pi;
	inclination = d2r(Theta[5]);
	tau = Theta[6]*kali::Day;
	alpha1 = -0.44;
	alpha2 = -0.44;
	period = kali::twoPi*sqrt(pow(a1 + a2, 3.0)/(kali::G*totalMass));
	epoch = 0.0;
	M = kali::twoPi*(epoch - tau)/period;
	nlopt::opt opt(nlopt::LN_COBYLA, 1);
	KeplersEqnData Data;
	Data.eccentricity = eccentricity;
	Data.M = M;
	KeplersEqnData *ptr2Data = &Data;
	opt.set_min_objective(KeplerEqn, ptr2Data);
	opt.set_xtol_rel(1e-16);
	std::vector<double> x(1);
	x[0] = 1.0;
	double minf;
	nlopt::result result = opt.optimize(x, minf);
	E = x[0];
	r1 = a1*(1.0 - eccentricity*cos(E)); // current distance of m1 from COM
	r2 = (m1*r1)/m2; // current distance of m2 from COM
	nu = 2.0*atan(eccentricityFactor*tan(E/2.0)); // current true anomoly of m1
	if (nu < 0.0) {
		nu += 2.0*kali::pi;
		}
	theta1 = nu + omega1;
	theta2 = nu + omega2;
	beta1 = sqrt(((kali::G*pow(m2, 2.0))/totalMass)*((2.0/r1) - (1.0/a1)))/kali::c;
	beta2 = sqrt(((kali::G*pow(m1, 2.0))/totalMass)*((2.0/r2) - (1.0/a2)))/kali::c;
	_radialBetaFactor1 = (((kali::twoPi/period)*a1)/(sqrt(1.0 - pow(eccentricity, 2.0))))*sin(inclination);
	_radialBetaFactor2 = (((kali::twoPi/period)*a2)/(sqrt(1.0 - pow(eccentricity, 2.0))))*sin(inclination);
	radialBeta1 = (_radialBetaFactor1*(cos(nu + omega1) + eccentricity*cos(omega1)))/kali::c;
	radialBeta2 = (_radialBetaFactor1*(cos(nu + omega2) + eccentricity*cos(omega2)))/kali::c;
	dF1 = (sqrt(1.0 - pow(beta1, 2.0)))/(1.0 - radialBeta1);
	dF2 = (sqrt(1.0 - pow(beta2, 2.0)))/(1.0 - radialBeta2);
	bF1 = pow(dF1, 3.0 - alpha1);
	bF2 = pow(dF2, 3.0 - alpha2);
	totalFlux = Theta[7];

} // setMBHBCARMA

double kali::MBHBCARMA::getEpoch() {return epoch/kali::Day;}

void kali::MBHBCARMA::setEpoch(double epochIn) {
	epoch = epochIn*kali::Day;
	(*this)();
	}

double kali::MBHBCARMA::getPeriod() {return period/kali::Day;}

double kali::MBHBCARMA::getA1() {return a1/kali::Parsec;}

double kali::MBHBCARMA::getA2() {return a2/kali::Parsec;}

double kali::MBHBCARMA::getM1() {return m1/(kali::SolarMass*1.0e6);}

double kali::MBHBCARMA::getM2() {return m2/(kali::SolarMass*1.0e6);}

double kali::MBHBCARMA::getM12() {return totalMass/(kali::SolarMass*1.0e6);}

double kali::MBHBCARMA::getM2OverM1() {return massRatio;}

double kali::MBHBCARMA::getRPeribothron1() {return rPeribothron1/kali::Parsec;}

double kali::MBHBCARMA::getRPeribothron2() {return rPeribothron2/kali::Parsec;}

double kali::MBHBCARMA::getRPeribothronTot() {return rPeribothronTot/kali::Parsec;}

double kali::MBHBCARMA::getRApobothron1() {return rApobothron1/kali::Parsec;}

double kali::MBHBCARMA::getRApobothron2() {return rApobothron2/kali::Parsec;}

double kali::MBHBCARMA::getRApobothronTot() {return rApobothronTot/kali::Parsec;}

double kali::MBHBCARMA::getRS1() {return rS1/kali::Parsec;}

double kali::MBHBCARMA::getRS2() {return rS2/kali::Parsec;}

double kali::MBHBCARMA::getEccentricity() {return eccentricity;}

double kali::MBHBCARMA::getOmega1() {return r2d(omega1);}

double kali::MBHBCARMA::getOmega2() {return r2d(omega2);}

double kali::MBHBCARMA::getInclination() {return r2d(inclination);}

double kali::MBHBCARMA::getTau() {return tau/kali::Day;}

double kali::MBHBCARMA::getMeanAnomoly() {return r2d(M);}

double kali::MBHBCARMA::getEccentricAnomoly() {return r2d(E);}

double kali::MBHBCARMA::getTrueAnomoly() {return r2d(nu);}

double kali::MBHBCARMA::getR1() {return r1/kali::Parsec;}

double kali::MBHBCARMA::getR2() {return r2/kali::Parsec;}

double kali::MBHBCARMA::getTheta1() {return r2d(theta1);}

double kali::MBHBCARMA::getTheta2() {return r2d(theta2);}

double kali::MBHBCARMA::getBeta1() {return beta1;}

double kali::MBHBCARMA::getBeta2() {return beta2;}

double kali::MBHBCARMA::getRadialBeta1() {return radialBeta1;}

double kali::MBHBCARMA::getRadialBeta2() {return radialBeta2;}

double kali::MBHBCARMA::getDopplerFactor1() {return dF1;}

double kali::MBHBCARMA::getDopplerFactor2() {return dF2;}

double kali::MBHBCARMA::getBeamingFactor1() {return bF1;}

double kali::MBHBCARMA::getBeamingFactor2() {return bF2;}

double kali::MBHBCARMA::aH(double sigmaStars) {
	double aHVal = (kali::G*reducedMass)/(4.0*pow((sigmaStars*kali::kms2ms), 2.0));
	return aHVal/kali::Parsec;
	}

double kali::MBHBCARMA::aGW(double sigmaStars, double rhoStars, double H) {
	double aGWVal = pow((64.0*pow(kali::G*reducedMass, 2.0)*totalMass*(kali::kms2ms*sigmaStars))/(5.0*H*pow(kali::c, 5.0)*(kali::SolarMassPerCubicParsec*rhoStars)), 0.2);
	return aGWVal/kali::Parsec;
	}

double kali::MBHBCARMA::durationInHardState(double sigmaStars, double rhoStars, double H) {
	double durationInHardStateVal = ((sigmaStars*kali::kms2ms)/(H*kali::G*(kali::SolarMassPerCubicParsec*rhoStars)*aGW(sigmaStars, rhoStars, H)));
	return durationInHardStateVal/kali::Day;
	}

double kali::MBHBCARMA::ejectedMass(double sigmaStars, double rhoStars, double H) {
	double ejectedMassVal = totalMass*log(aH(sigmaStars)/aGW(sigmaStars, rhoStars, H));
	return ejectedMassVal/(kali::SolarMass*1.0e6);
	}

void kali::MBHBCARMA::operator()() {
	M = kali::twoPi*(epoch - tau)/period;
	nlopt::opt opt(nlopt::LN_COBYLA, 1);
	KeplersEqnData Data;
	Data.eccentricity = eccentricity;
	Data.M = M;
	KeplersEqnData *ptr2Data = &Data;
	opt.set_min_objective(KeplerEqn, ptr2Data);
	opt.set_xtol_rel(1e-16);
	std::vector<double> x(1);
	x[0] = 1.0;
	double minf;
	nlopt::result result = opt.optimize(x, minf);
	E = x[0];
	r1 = a1*(1.0 - eccentricity*cos(E)); // current distance of m1 from COM
	r2 = (m1*r1)/m2; // current distance of m2 from COM
	nu = 2.0*atan(eccentricityFactor*tan(E/2.0)); // current true anomoly of m1
	if (nu < 0.0) {
		nu += 2.0*kali::pi;
		}
	theta1 = nu + omega1;
	theta2 = nu + omega2;
	beta1 = sqrt(((kali::G*pow(m2, 2.0))/totalMass)*((2.0/r1) - (1.0/a1)))/kali::c;
	beta2 = sqrt(((kali::G*pow(m1, 2.0))/totalMass)*((2.0/r2) - (1.0/a2)))/kali::c;
	radialBeta1 = (_radialBetaFactor1*(cos(nu + omega1) + eccentricity*cos(omega1)))/kali::c;
	radialBeta2 = (_radialBetaFactor1*(cos(nu + omega2) + eccentricity*cos(omega2)))/kali::c;
	dF1 = (sqrt(1.0 - pow(beta1, 2.0)))/(1.0 - radialBeta1);
	dF2 = (sqrt(1.0 - pow(beta2, 2.0)))/(1.0 - radialBeta2);
	bF1 = pow(dF1, 3.0 - alpha1);
	bF2 = pow(dF2, 3.0 - alpha2);
} // operator()

void kali::MBHBCARMA::solveMBHBCARMA() {
	#if (defined DEBUG_SOLVEMBHBCARMA_F) || (defined DEBUG_SOLVEMBHBCARMA_Q)
	   int threadNum = omp_get_thread_num();
	#endif

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; Address of System: %p\n",threadNum,this);
	printf("\n");
	#endif

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; w (Before)\n",threadNum);
	viewMatrix(p,1,w);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; expw = exp(w) (Before)\n",threadNum);
	viewMatrix(p,p,expw);
	printf("\n");
	#endif

	#pragma omp simd
	for (int i = 0; i < p; ++i) {
		expw[i + i*p] = exp(dt*w[i]);
		}

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; w (After)\n",threadNum);
	viewMatrix(p,1,w);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; expw = exp(w) (After)\n",threadNum);
	viewMatrix(p,p,expw);
	printf("\n");
	#endif

	complex<double> alpha = kali::complexOne, beta = kali::complexZero;

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; vr (Before)\n",threadNum);
	viewMatrix(p,p,vr);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; expw (Before)\n",threadNum);
	viewMatrix(p,p,expw);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; ACopy = vr*expw (Before)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, expw, p, &beta, ACopy, p);

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; vr (After)\n",threadNum);
	viewMatrix(p,p,vr);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; expw (After)\n",threadNum);
	viewMatrix(p,p,expw);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; ACopy = vr*expw (After)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	#endif

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; ACopy (Before)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; vrInv (Before)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; AScratch = ACopy*vrInv (Before)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	#endif

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, ACopy, p, vrInv, p, &beta, AScratch, p);

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; ACopy (After)\n",threadNum);
	viewMatrix(p,p,ACopy);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; vrInv (After)\n",threadNum);
	viewMatrix(p,p,vrInv);
	printf("\n");
	printf("solveMBHBCARMA - threadNum: %d; AScratch = aCopy*vrInv (After)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	#endif

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveDLM - threadNum: %d; AScratch (Before)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	printf("solveDLM - threadNum: %d; F = AScratch.real() (Before)\n",threadNum);
	viewMatrix(p,p,F);
	printf("\n");
	#endif

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			F[rowCtr + colCtr*p] = AScratch[rowCtr + colCtr*p].real();
			}
		}

	#ifdef DEBUG_SOLVEMBHBCARMA_F
	printf("solveMBHBCARMA - threadNum: %d; walkerPos: ",threadNum);
	for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
		printf("%+8.7e ",Theta[dimNum]);
		}
	printf("\n");
	printf("solveDLM - threadNum: %d; AScratch (After)\n",threadNum);
	viewMatrix(p,p,AScratch);
	printf("\n");
	printf("solveDLM - threadNum: %d; F = AScratch.real() (After)\n",threadNum);
	viewMatrix(p,p,F);
	printf("\n");
	#endif

	for (int colNum = 0; colNum < p; ++colNum) {
		#pragma omp simd
		for (int rowNum = 0; rowNum < p; ++rowNum) {
			AScratch[rowNum + p*colNum] = C[rowNum + p*colNum]*(exp((kali::complexOne*dt)*(w[rowNum] + w[colNum])) - kali::complexOne)*(kali::complexOne/(w[rowNum] + w[colNum])); // AScratch[i,j] = (C[i,j]*(exp((lambda[i] + lambda[j])*dt) - 1))/(lambda[i] + lambda[j])
			ACopy[rowNum + p*colNum] = C[rowNum + p*colNum]*( - kali::complexOne)*(kali::complexOne/(w[rowNum] + w[colNum])); // ACopy[i,j] = = (C[i,j]*( - 1/(lambda[i] + lambda[j]))
		}
	}

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, AScratch, p, &beta, AScratch2, p); // AScratch2 = vr*AScratch
	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, &alpha, AScratch2, p, vr, p, &beta, AScratch, p); // AScratch = AScratch2*trans(vr)

	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, ACopy, p, &beta, AScratch2, p); // AScratch2 = vr*ACopy
	cblas_zgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, &alpha, AScratch2, p, vr, p, &beta, ACopy, p); // Sigma = AScratch2*trans(vr)

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			Q[rowCtr + colCtr*p] = AScratch[rowCtr + colCtr*p].real();
			Sigma[rowCtr + colCtr*p] = ACopy[rowCtr + colCtr*p].real();
			}
		}

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			T[rowCtr + colCtr*p] = Q[rowCtr + colCtr*p];
			}
		}

	char uplo = 'U';
	int n = p, lda = p;
	lapack_int YesNo;
	//YesNo = LAPACKE_dpotrf(LAPACK_COL_MAJOR, 'U', p, T, p);
	dpotrf(&uplo, &n, T, &lda, &YesNo);

} // solveMBHBCARMA

void kali::MBHBCARMA::resetState(double InitUncertainty) {

	#ifdef DEBUG_RESETSTATE
	int threadNum = omp_get_thread_num();
	printf("resetState - threadNum: %d; Address of System: %p\n",threadNum,this);
	printf("\n");
	#endif

	for (int i = 0; i < p; i++) {
		X[i] = 0.0;
		XMinus[i] = 0.0;
		VScratch[i] = 0.0;
		#pragma omp simd
		for (int j = 0; j < p; j++) {
			P[i*p+j] = 0.0;
			PMinus[i*p+j] = 0.0;
			MScratch[i*p+j] = 0.0;
			}
		P[i*p+i] = InitUncertainty;
		}
	}

void kali::MBHBCARMA::resetState() {

	// Copy P and reset the other matrices
	for (int colCtr = 0; colCtr < p; ++colCtr) {
		X[colCtr] = 0.0;
		XMinus[colCtr] = 0.0;
		VScratch[colCtr] = 0.0;
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			PMinus[rowCtr + colCtr*p] = 0.0;
			MScratch[rowCtr + colCtr*p] = 0.0;
			P[rowCtr + colCtr*p] = Sigma[rowCtr + colCtr*p];
			}
		}

	#ifdef DEBUG_RESETSTATE
	printf("resetState - threadNum: %d; P\n",threadNum);
	viewMatrix(p,p,P);
	printf("\n");
	#endif

	}

void kali::MBHBCARMA::burnSystem(int numBurn, unsigned int burnSeed, double* burnRand) {
	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	VSLStreamStatePtr burnStream __attribute__((aligned(64)));
	vslNewStream(&burnStream, VSL_BRNG_SFMT19937, burnSeed);
	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		VScratch[rowCtr] = 0.0;
		}
	vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, burnStream, numBurn, burnRand, p, VSL_MATRIX_STORAGE_FULL, VScratch, T);
	vslDeleteStream(&burnStream);

	for (int i = 0; i < numBurn; ++i) {
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1); // VScratch = F*X
		cblas_dcopy(p, VScratch, 1, X, 1); // X = VScratch
		cblas_daxpy(p, 1.0, &burnRand[i*p], 1, X, 1); // X = w*D + X
		}
	}

void kali::MBHBCARMA::beamSystem(LnLikeData *ptr2Data) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
    double fracIntrinsicVar = Data.fracIntrinsicVar;
    double fracNoiseToSignal = Data.fracNoiseToSignal;
	double tolIR = Data.tolIR;
    double startT = Data.startT;
	double *t = Data.t;
	double *x = Data.x;
	double *mask = Data.mask;

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);

	double absMeanFlux = totalFlux; //absIntrinsicVar/fracIntrinsicVar;
	double absFlux = 0.0, noiseLvl = 0.0, t_incr = 0.0, fracChange = 0.0;
    #ifdef DEBUG_BEAMSYSTEM
        printf("\n");
        printf("absMeanFlux: %+e\n", absMeanFlux);
    #endif

    setEpoch(t[0]);
	x[0] = absMeanFlux*getBeamingFactor2();
    #ifdef DEBUG_BEAMSYSTEM
        printf("absMeanFlux: %+e\n", absMeanFlux);
        printf("getBeamingFactor2(): %+e\n", getBeamingFactor2());
        printf("x[0]: %+e\n", x[0]);
    #endif

	for (int i = 1; i < numCadences; ++i) {
        setEpoch(t[i]);
		t_incr = t[i] - t[i - 1];
		fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));

		if (fracChange > tolIR) {
			dt = t_incr;
			}

		x[i] = absMeanFlux*getBeamingFactor2();
        #ifdef DEBUG_BEAMSYSTEM
            printf("absMeanFlux: %+e\n", absMeanFlux);
            printf("getBeamingFactor2(): %+e\n", getBeamingFactor2());
            printf("x[%d]: %+e\n", i, x[i]);
        #endif
		}
	}

void kali::MBHBCARMA::simulateSystem(LnLikeData *ptr2Data, unsigned int distSeed, double *distRand) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
    double fracIntrinsicVar = Data.fracIntrinsicVar;
    double fracNoiseToSignal = Data.fracNoiseToSignal;
	double tolIR = Data.tolIR;
    double startT = Data.startT;
	double *t = Data.t;
	double *x = Data.x;
	double *mask = Data.mask;

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	VSLStreamStatePtr distStream __attribute__((aligned(64)));
	vslNewStream(&distStream, VSL_BRNG_SFMT19937, distSeed);

    double absIntrinsicVar = sqrt(Sigma[0]);
	double absMeanFlux = totalFlux; //absIntrinsicVar/fracIntrinsicVar;
	double absFlux = 0.0, noiseLvl = 0.0, t_incr = 0.0, fracChange = 0.0;
    #ifdef DEBUG_SIMULATESYSTEM
        printf("\n");
        printf("absIntrinsicVar: %+e; absMeanFlux: %+e\n", absIntrinsicVar, absMeanFlux);
    #endif

	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		VScratch[rowCtr] = 0.0;
		}
	vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, distStream, 1, &distRand[0], p, VSL_MATRIX_STORAGE_FULL, VScratch, T);

    setEpoch(t[0]);
	cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1); // VScratch = F*x
	cblas_dcopy(p, VScratch, 1, X, 1); // X = VScratch
	cblas_daxpy(p, 1.0, &distRand[0], 1, X, 1);
	x[0] = (X[0] + absMeanFlux)*getBeamingFactor2();
    #ifdef DEBUG_SIMULATESYSTEM
        printf("X[0]: %+e\n", X[0]);
        printf("absMeanFlux: %+e\n", absMeanFlux);
        printf("getBeamingFactor2(): %+e\n", getBeamingFactor2());
        printf("x[0]: %+e\n", x[0]);
    #endif

	for (int i = 1; i < numCadences; ++i) {
        setEpoch(t[i]);
		t_incr = t[i] - t[i - 1];
		fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));

		if (fracChange > tolIR) {
			dt = t_incr;
			solveMBHBCARMA();
			}

		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			VScratch[rowCtr] = 0.0;
			}
		vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, distStream, 1, &distRand[i*p], p, VSL_MATRIX_STORAGE_FULL, VScratch, T);

		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1);
		cblas_dcopy(p, VScratch, 1, X, 1);
		cblas_daxpy(p, 1.0, &distRand[i*p], 1, X, 1);
		x[i] = (X[0] + absMeanFlux)*getBeamingFactor2();
        #ifdef DEBUG_SIMULATESYSTEM
            printf("X[0]: %+e\n", X[0]);
            printf("absMeanFlux: %+e\n", absMeanFlux);
            printf("getBeamingFactor2(): %+e\n", getBeamingFactor2());
            printf("x[%d]: %+e\n", i, x[i]);
        #endif
		}

	vslDeleteStream(&distStream);
	}

/*void kali::MBHBCARMA::extendSystem(LnLikeData *ptr2Data, unsigned int distSeed, double *distRand) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	int cadenceNum = Data.cadenceNum;
	double tolIR = Data.tolIR;
	double *t = Data.t;
	double *x = Data.x;
	double *mask = Data.mask;

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	VSLStreamStatePtr distStream __attribute__((aligned(64)));
	vslNewStream(&distStream, VSL_BRNG_SFMT19937, distSeed);

	double t_incr = 0.0, fracChange = 0.0;
	int startCadence = cadenceNum + 1;

	#pragma omp simd
	for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
		VScratch[rowCtr] = 0.0;
		}
	vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, distStream, 1, &distRand[0], p, VSL_MATRIX_STORAGE_FULL, VScratch, T);

	cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1); // VScratch = F*x
	cblas_dcopy(p, VScratch, 1, X, 1); // X = VScratch
	cblas_daxpy(p, 1.0, &distRand[0], 1, X, 1);
	x[startCadence] = X[0];

	for (int i = startCadence + 1; i < numCadences; ++i) {

		t_incr = t[i] - t[i - 1];
		fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));

		if (fracChange > tolIR) {
			dt = t_incr;
			solveMBHBCARMA();
			}

		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			VScratch[rowCtr] = 0.0;
			}
		vdRngGaussianMV(VSL_RNG_METHOD_GAUSSIANMV_ICDF, distStream, 1, &distRand[(i - startCadence - 1)*p], p, VSL_MATRIX_STORAGE_FULL, VScratch, T);

		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, VScratch, 1);
		cblas_dcopy(p, VScratch, 1, X, 1);
		cblas_daxpy(p, 1.0, &distRand[(i - startCadence - 1)*p], 1, X, 1);
		x[i] = X[0];
		}

	vslDeleteStream(&distStream);
	Data.cadenceNum = numCadences - 1;
}*/

double kali::MBHBCARMA::getIntrinsicVar() {
	return sqrt(Sigma[0]);
	}

double kali::MBHBCARMA::getMeanFlux(LnLikeData *ptr2Data) {
	kali::LnLikeData Data = *ptr2Data;

	double fracIntrinsicVar = Data.fracIntrinsicVar;

	double intrinsicVar = sqrt(Sigma[0]);
	return intrinsicVar/fracIntrinsicVar;
	}

void kali::MBHBCARMA::observeNoise(LnLikeData *ptr2Data, unsigned int noiseSeed, double* noiseRand) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	double fracIntrinsicVar = Data.fracIntrinsicVar;
	double fracNoiseToSignal = Data.fracNoiseToSignal;
	double *t = Data.t;
	double *x = Data.x;
	double *y = Data.y;
	double *yerr = Data.yerr;
	double *mask = Data.mask;

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	VSLStreamStatePtr noiseStream __attribute__((aligned(64)));
	vslNewStream(&noiseStream, VSL_BRNG_SFMT19937, noiseSeed);
	double noiseLvl = 0.0;

	for (int i = 0; i < numCadences; ++i) {
		noiseLvl = fracNoiseToSignal*x[i];
		vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_ICDF, noiseStream, 1, &noiseRand[i], 0.0, noiseLvl);
		y[i] = x[i] + noiseRand[i];
		yerr[i] = noiseLvl;
		}
	vslDeleteStream(&noiseStream);
	}

/*void kali::MBHBCARMA::extendObserveNoise(LnLikeData *ptr2Data, unsigned int noiseSeed, double* noiseRand) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	int cadenceNum = Data.cadenceNum;
	double fracIntrinsicVar = Data.fracIntrinsicVar;
	double fracNoiseToSignal = Data.fracNoiseToSignal;
	double *t = Data.t;
	double *x = Data.x;
	double *y = Data.y;
	double *yerr = Data.yerr;
	double *mask = Data.mask;

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	VSLStreamStatePtr noiseStream __attribute__((aligned(64)));
	vslNewStream(&noiseStream, VSL_BRNG_SFMT19937, noiseSeed);

	double absIntrinsicVar = sqrt(Sigma[0]);
	double absMeanFlux = absIntrinsicVar/fracIntrinsicVar;
	double absFlux = 0.0, noiseLvl = 0.0;
	int startCadence = cadenceNum + 1;
	for (int i = startCadence; i < numCadences; ++i) {
		absFlux = absMeanFlux + x[i];
		noiseLvl = fracNoiseToSignal*absFlux;
		vdRngGaussian(VSL_RNG_METHOD_GAUSSIAN_ICDF, noiseStream, 1, &noiseRand[i - startCadence], 0.0, noiseLvl);
		y[i] = absFlux + noiseRand[i - startCadence];
		yerr[i] = noiseLvl;
		}
	vslDeleteStream(&noiseStream);
	Data.cadenceNum = numCadences - 1;
}*/

double kali::MBHBCARMA::computeLnLikelihood(LnLikeData *ptr2Data) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	double tolIR = Data.tolIR;
	double *t = Data.t;
	double *y = Data.y;
	double *yerr = Data.yerr;
	double *mask = Data.mask;
    double startT = Data.startT*kali::Day;
	double maxDouble = numeric_limits<double>::max();

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	double t_incr = 0.0, LnLikelihood = 0.0, ptCounter = 0.0, v = 0.0, S = 0.0, SInv = 0.0, fracChange = 0.0, Contrib = 0.0;

    setEpoch(t[0]);
	H[0] = mask[0];
	R[0] = yerr[0]*yerr[0]; // Heteroskedastic errors
	cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;

    double yVal = (y[0]/getBeamingFactor2()) - totalFlux;
	v = mask[0]*(yVal - H[0]*XMinus[0]); // Compute v = y - H*X
	cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
	S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R

	#ifdef DEBUG_COMPUTELNLIKELIHOOD
		printf("y[%d]: %e\n", 0, y[0]);
		printf("yerr[%d]: %e\n", 0, yerr[0]);
		printf("mask[%d]: %e\n", 0, mask[0]);
        printf("totalFlux: %e\n", totalFlux);
        printf("getBeamingFactor2(): %e\n", getBeamingFactor2());
        printf("yVal[%d]: %e\n", 0, yVal);
        printf("H[0]*XMinus[0]: %+e\n", H[0]*XMinus[0]);
		printf("v[%d]: %e\n", 0, v);
		printf("S[%d]: %e\n", 0, S);
	#endif

	SInv = 1.0/S;
	cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
	for (int colCounter = 0; colCounter < p; colCounter++) {
		#pragma omp simd
		for (int rowCounter = 0; rowCounter < p; rowCounter++) {
			MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
			}
		}
	cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
	cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, yVal, VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
	cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
	for (int colCounter = 0; colCounter < p; colCounter++) {
		#pragma omp simd
		for (int rowCounter = 0; rowCounter < p; rowCounter++) {
			P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
			}
		}
	Contrib = mask[0]*(-0.5*SInv*pow(v,2.0) -0.5*log2(S)/kali::log2OfE);

	#ifdef DEBUG_COMPUTELNLIKELIHOOD
		printf("Contrib[%d]: %e\n", 0, Contrib);
	#endif

	LnLikelihood = LnLikelihood + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
	ptCounter = ptCounter + 1*static_cast<int>(mask[0]);
	for (int i = 1; i < numCadences; i++) {
        setEpoch(t[i]);
        t_incr = t[i] - t[i - 1];
		fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));
		if (fracChange > tolIR) {
			dt = t_incr;
			solveMBHBCARMA();
			}
		H[0] = mask[i];
		R[0] = yerr[i]*yerr[i]; // Heteroskedastic errors
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;
        yVal = (y[i]/getBeamingFactor2()) - totalFlux;
        v = mask[i]*(yVal - H[0]*XMinus[0]); // Compute v = y - H*X
		cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
		S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R

		#ifdef DEBUG_COMPUTELNLIKELIHOOD
			if (i < MAXPRINT) {
				printf("y[%d]: %e\n", i, y[i]);
				printf("yerr[%d]: %e\n", i, yerr[i]);
				printf("mask[%d]: %e\n", i, mask[i]);
                printf("totalFlux: %e\n", totalFlux);
                printf("getBeamingFactor2(): %e\n", getBeamingFactor2());
                printf("yVal[%d]: %e\n", 0, yVal);
                printf("H[0]*XMinus[0]: %+e\n", H[0]*XMinus[0]);
				printf("v[%d]: %e\n", i, v);
				printf("S[%d]: %e\n", i, S);
				}
		#endif

		SInv = 1.0/S;
		cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
		for (int colCounter = 0; colCounter < p; colCounter++) {
			#pragma omp simd
			for (int rowCounter = 0; rowCounter < p; rowCounter++) {
				MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
				}
			}
		cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, yVal, VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
		cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
		for (int colCounter = 0; colCounter < p; colCounter++) {
			#pragma omp simd
			for (int rowCounter = 0; rowCounter < p; rowCounter++) {
				P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
				}
			}
		Contrib = mask[i]*(-0.5*SInv*pow(v,2.0) -0.5*log2(S)/kali::log2OfE);

		#ifdef DEBUG_COMPUTELNLIKELIHOOD
			if (i < MAXPRINT) {
				printf("Contrib[%d]: %e\n", i, Contrib);
			}
		#endif

		LnLikelihood = LnLikelihood + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
		ptCounter = ptCounter + 1*static_cast<int>(mask[0]);
		}
    #ifdef DEBUG_COMPUTELNLIKELIHOOD
    	printf("LnLike: %e\n", LnLikelihood);
    #endif
	LnLikelihood += -0.5*ptCounter*kali::log2Pi;

	#ifdef DEBUG_COMPUTELNLIKELIHOOD
		printf("LnLike: %e\n", LnLikelihood);
	#endif

	Data.cadenceNum = numCadences - 1;
	Data.currentLnLikelihood = LnLikelihood;
	return LnLikelihood;
	}

/*double kali::MBHBCARMA::updateLnLikelihood(LnLikeData *ptr2Data) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	int cadenceNum = Data.cadenceNum;
	double currentLnLikelihood = Data.currentLnLikelihood;
	double tolIR = Data.tolIR;
	double *t = Data.t;
	double *y = Data.y;
	double *yerr = Data.yerr;
	double *mask = Data.mask;
	double maxSigma = Data.maxSigma;
	double maxTimescale = Data.maxTimescale;
	double maxDouble = numeric_limits<double>::max();

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	double t_incr = 0.0, LnLikelihood = 0.0, ptCounter = 0.0, v = 0.0, S = 0.0, SInv = 0.0, fracChange = 0.0, Contrib = 0.0;

	int startCadence = cadenceNum + 1;
	H[0] = mask[startCadence];
	R[0] = yerr[startCadence]*yerr[startCadence]; // Heteroskedastic errors
	cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;
	v = mask[startCadence]*(y[startCadence] - H[0]*XMinus[0]); // Compute v = y - H*X
	cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
	S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R

	#ifdef DEBUG_COMPUTELNLIKELIHOOD
		printf("y[%d]: %e\n", startCadence, y[startCadence]);
		printf("yerr[%d]: %e\n", startCadence, yerr[startCadence]);
		printf("mask[%d]: %e\n", startCadence, mask[startCadence]);
		printf("v[%d]: %e\n", startCadence, v);
		printf("S[%d]: %e\n", startCadence, S);
	#endif

	SInv = 1.0/S;
	cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
	for (int colCounter = 0; colCounter < p; colCounter++) {
		#pragma omp simd
		for (int rowCounter = 0; rowCounter < p; rowCounter++) {
			MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
			}
		}
	cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
	cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, y[0], VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
	cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
	for (int colCounter = 0; colCounter < p; colCounter++) {
		#pragma omp simd
		for (int rowCounter = 0; rowCounter < p; rowCounter++) {
			P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
			}
		}
	Contrib = mask[startCadence]*(-0.5*SInv*pow(v,2.0) -0.5*log2(S)/kali::log2OfE);

	#ifdef DEBUG_COMPUTELNLIKELIHOOD
		printf("Contrib[%d]: %e\n", startCadence, Contrib);
	#endif

	LnLikelihood = LnLikelihood + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
	ptCounter = ptCounter + 1*static_cast<int>(mask[startCadence]);
	for (int i = startCadence + 1; i < numCadences; i++) {
		t_incr = t[i] - t[i - 1];
		fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));
		if (fracChange > tolIR) {
			dt = t_incr;
			solveMBHBCARMA();
			}
		H[0] = mask[i];
		R[0] = yerr[i]*yerr[i]; // Heteroskedastic errors
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;
		v = mask[i]*(y[i] - H[0]*XMinus[0]); // Compute v = y - H*X
		cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
		S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R

		#ifdef DEBUG_COMPUTELNLIKELIHOOD
			if (i < MAXPRINT) {
				printf("y[%d]: %e\n", i, y[i]);
				printf("yerr[%d]: %e\n", i, yerr[i]);
				printf("mask[%d]: %e\n", i, mask[i]);
				printf("v[%d]: %e\n", i, v);
				printf("S[%d]: %e\n", i, S);
				}
		#endif

		SInv = 1.0/S;
		cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
		for (int colCounter = 0; colCounter < p; colCounter++) {
			#pragma omp simd
			for (int rowCounter = 0; rowCounter < p; rowCounter++) {
				MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
				}
			}
		cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, y[i], VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
		cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
		for (int colCounter = 0; colCounter < p; colCounter++) {
			#pragma omp simd
			for (int rowCounter = 0; rowCounter < p; rowCounter++) {
				P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
				}
			}
		Contrib = mask[i]*(-0.5*SInv*pow(v,2.0) -0.5*log2(S)/kali::log2OfE);

		#ifdef DEBUG_COMPUTELNLIKELIHOOD
			if (i < MAXPRINT) {
				printf("Contrib[%d]: %e\n", i, Contrib);
			}
		#endif

		LnLikelihood = LnLikelihood + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
		ptCounter = ptCounter + 1*static_cast<int>(mask[i]);
		}
	LnLikelihood += -0.5*ptCounter*kali::log2Pi;

	#ifdef DEBUG_COMPUTELNLIKELIHOOD
		printf("LnLike: %e\n", LnLikelihood);
	#endif

	Data.cadenceNum = numCadences - 1;
	currentLnLikelihood += LnLikelihood;
	Data.currentLnLikelihood += LnLikelihood;

	return currentLnLikelihood;
}*/


double kali::MBHBCARMA::computeLnPrior(LnLikeData *ptr2Data) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	double currentLnPrior = Data.currentLnPrior;
	double tolIR = Data.tolIR;
	double *t = Data.t;
    double meandt = Data.meandt*kali::Day;
    double T = (t[numCadences-1] - t[0])*kali::Day;
	double *y = Data.y;
	double *yerr = Data.yerr;
	double *mask = Data.mask;
    double startT = Data.startT*kali::Day;
	double maxSigma = Data.maxSigma;
	double minTimescale = Data.minTimescale;
	double maxTimescale = Data.maxTimescale;
    double lowestFlux = Data.lowestFlux;
	double highestFlux = Data.highestFlux;
    double periodCenter = Data.periodCenter;
    double periodWidth = Data.periodWidth;
    double fluxCenter = Data.fluxCenter;
    double fluxWidth = Data.fluxWidth;
	double maxDouble = numeric_limits<double>::max();

	#ifdef DEBUG_COMPUTELNPRIOR
	   int threadNum = omp_get_thread_num();
	#endif

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	double LnPrior = 0.0, timescale = 0.0, timescaleOsc = 0.0;

	#ifdef DEBUG_COMPUTELNPRIOR
	    printf("computeLnPrior - threadNum: %d; maxSigma:           %+4.3e\n", threadNum, maxSigma);
	    printf("computeLnPrior - threadNum: %d; sqrt(Sigma[0]):     %+4.3e\n", threadNum, sqrt(Sigma[0]));
	#endif

	if (sqrt(Sigma[0]) > maxSigma) {
		LnPrior = -kali::infiniteVal;
		}

    #ifdef DEBUG_COMPUTELNPRIOR
        printf("computeLnPrior - threadNum: %d; LnPrior after Sigma: %+4.3e\n",threadNum,LnPrior);
    #endif

	for (int i = 0; i < p; ++i) {
		timescale = fabs(1.0/(CARw[i].real()));
		timescaleOsc = fabs((2.0*kali::pi)/(CARw[i].imag()));

		#ifdef DEBUG_COMPUTELNPRIOR
		    printf("computeLnPrior - threadNum: %d; minTimescale:       %+4.3e\n", threadNum, minTimescale);
		    printf("computeLnPrior - threadNum: %d; maxTimescale:       %+4.3e\n", threadNum, maxTimescale);
		    printf("computeLnPrior - threadNum: %d; CARw:               %+4.3e %+4.3e\n", threadNum, CARw[i].real(), CARw[i].imag());
		    printf("computeLnPrior - threadNum: %d; timescale (CAR):    %+4.3e\n", threadNum, timescale);
		    printf("computeLnPrior - threadNum: %d; timescaleOsc (CAR): %+4.3e\n", threadNum, timescaleOsc);
		#endif

		if (timescale < minTimescale) {
			LnPrior = -kali::infiniteVal;
			}

		if (timescale > maxTimescale) {
			LnPrior = -kali::infiniteVal;
			}

		if (timescaleOsc > 0.0) {
			if (timescaleOsc < minTimescale) {
				LnPrior = -kali::infiniteVal;
				}
			}

		}

    #ifdef DEBUG_COMPUTELNPRIOR
        printf("computeLnPrior - threadNum: %d; LnPrior after CAR: %+4.3e\n",threadNum,LnPrior);
    #endif

	for (int i = 0; i < q; ++i) {
		timescale = fabs(1.0/(CMAw[i].real()));
		timescaleOsc = fabs((2.0*kali::pi)/(CMAw[i].imag()));

		#ifdef DEBUG_COMPUTELNPRIOR
            printf("computeLnPrior - threadNum: %d; minTimescale:       %+4.3e\n", threadNum, minTimescale);
            printf("computeLnPrior - threadNum: %d; maxTimescale:       %+4.3e\n", threadNum, maxTimescale);
		    printf("computeLnPrior - threadNum: %d; CMAw:               %+4.3e %+4.3e\n", threadNum, CMAw[i].real(), CMAw[i].imag());
		    printf("computeLnPrior - threadNum: %d; timescale (CMA):    %+4.3e\n", threadNum, timescale);
		    printf("computeLnPrior - threadNum: %d; timescaleOsc(CMA):  %+4.3e\n", threadNum, timescaleOsc);
		#endif

		if (timescale < minTimescale) {
			LnPrior = -kali::infiniteVal;
			}

		if (timescale > maxTimescale) {
			LnPrior = -kali::infiniteVal;
			}

		if (timescaleOsc > 0.0) {
			if (timescaleOsc < minTimescale) {
				LnPrior = -kali::infiniteVal;
				}
			}

		}

    #ifdef DEBUG_COMPUTELNPRIOR
	    printf("computeLnPrior - threadNum: %d; LnPrior after CMA: %+4.3e\n",threadNum,LnPrior);
    #endif

    if (m1 < 1.0e4*kali::SolarMass) {
		LnPrior = -infiniteVal; // Restrict m1 > 10^4 M_Sum
		}
	if (m1 > 1.0e10*kali::SolarMass) {
		LnPrior = -infiniteVal; // Restrict m1 < 10^10 M_Sun
		}

    #ifdef DEBUG_COMPUTELNPRIOR
        printf("computeLnPrior - threadNum: %d; LnPrior after m1: %+4.3e\n",threadNum,LnPrior);
    #endif

    if (m2 < 1.0e4*kali::SolarMass) {
		LnPrior = -infiniteVal; // Restrict m2 > 10^4 M_Sum
		}
	if (m2 > m1) {
		LnPrior = -infiniteVal; // Restrict m1 >= m2
		}

    #ifdef DEBUG_COMPUTELNPRIOR
        printf("computeLnPrior - threadNum: %d; LnPrior after m2: %+4.3e\n",threadNum,LnPrior);
    #endif

    if (tau < startT) {
		LnPrior = -infiniteVal; // Restrict tau to startT < tau < startT + period
		}
	if (tau > startT + period) {
		LnPrior = -infiniteVal; // Restrict tau to startT < tau < startT + period
		}

    #ifdef DEBUG_COMPUTELNPRIOR
        printf("computeLnPrior - threadNum: %d; LnPrior after tau: %+4.3e\n",threadNum,LnPrior);
    #endif

    if (totalFlux < lowestFlux) {
		LnPrior = -infiniteVal; // The total flux cannot be smaller than the smallest flux in the LC
		}
	if (totalFlux > highestFlux) {
		LnPrior = -infiniteVal; // The total flux cannot be bigger than the biggest flux in the LC
		}

    #ifdef DEBUG_COMPUTELNPRIOR
        printf("computeLnPrior - threadNum: %d; LnPrior after flux: %+4.3e\n",threadNum,LnPrior);
    #endif

	if (period < 2.0*dt) {
		LnPrior = -infiniteVal; // Cut all all configurations where the inferred period is too short!
		}
	if (period > 10.0*T) {
		LnPrior = -infiniteVal; // Cut all all configurations where the inferred period is too short!
		}

    #ifdef DEBUG_COMPUTELNPRIOR
        printf("computeLnPrior - threadNum: %d; LnPrior after period: %+4.3e\n",threadNum,LnPrior);
    #endif

    //double periodPrior = -0.5*kali::log2Pi -log2(periodWidth*kali::Day)/kali::log2OfE -0.5*pow(((periodCenter*kali::Day) - period)/(periodWidth*kali::Day), 2.0); // Prior on period
    //double totalFluxPrior = -0.5*kali::log2Pi -log2(fluxWidth)/kali::log2OfE -0.5*pow((fluxCenter - totalFlux)/fluxWidth, 2.0); // Prior on totalFlux
    double periodPrior = -0.5*pow(((periodCenter*kali::Day) - period)/(periodWidth*kali::Day), 2.0); // Prior on period
    double totalFluxPrior = -0.5*pow((fluxCenter - totalFlux)/fluxWidth, 2.0); // Prior on totalFlux
    LnPrior += periodPrior + totalFluxPrior;

    #ifdef DEBUG_COMPUTELNPRIOR
    	printf("computeLnPrior - threadNum: %d; totalFlux: %+e\n", threadNum, totalFlux);
        printf("computeLnPrior - threadNum: %d; lowestFlux: %+e\n", threadNum, lowestFlux);
        printf("computeLnPrior - threadNum: %d; highestFlux: %+e\n", threadNum, highestFlux);
        printf("computeLnPrior - threadNum: %d; period: %+e\n", threadNum, period);
        printf("computeLnPrior - threadNum: %d; 2.0*dt: %+e\n", threadNum, 2.0*dt);
        printf("computeLnPrior - threadNum: %d; 10.0*T: %+e\n", threadNum, 10.0*T);
        printf("computeLnPrior - threadNum: %d; period: %+e\n", threadNum, period);
        printf("computeLnPrior - threadNum: %d; periodCenter: %+e\n", threadNum, periodCenter*kali::Day);
        printf("computeLnPrior - threadNum: %d; periodWidth: %+e\n", threadNum, periodWidth*kali::Day);
        //printf("computeLnPrior - threadNum: %d;                                                                -0.5 ln 2pi: %+e\n", threadNum, -0.5*kali::log2Pi);
        //printf("computeLnPrior - threadNum: %d;                                 -log2(periodWidth*kali::Day)/kali::log2OfE: %+e\n", threadNum, -log2(periodWidth*kali::Day)/kali::log2OfE);
        //printf("computeLnPrior - threadNum: %d;                ((periodCenter*kali::Day) - period)/(periodWidth*kali::Day): %+e\n", threadNum, ((periodCenter*kali::Day) - period)/(periodWidth*kali::Day));
        //printf("computeLnPrior - threadNum: %d; -0.5*pow(((periodCenter*kali::Day) - period)/(periodWidth*kali::Day), 2.0): %+e\n", threadNum, -0.5*pow(((periodCenter*kali::Day) - period)/(periodWidth*kali::Day), 2.0));
        printf("computeLnPrior - threadNum: %d; periodPrior: %+e\n", threadNum, periodPrior);
        printf("computeLnPrior - threadNum: %d; totalFlux: %+e\n", threadNum, totalFlux);
        printf("computeLnPrior - threadNum: %d; fluxCenter: %+e\n", threadNum, fluxCenter);
        printf("computeLnPrior - threadNum: %d; fluxWidth: %+e\n", threadNum, fluxWidth);
        printf("computeLnPrior - threadNum: %d; totalFluxPrior: %+e\n", threadNum, totalFluxPrior);
	    printf("computeLnPrior - threadNum: %d; LnPrior: %+4.3e\n",threadNum,LnPrior);
	    printf("\n");
	#endif

	Data.currentLnPrior = LnPrior;

	return LnPrior;
	}

/*void kali::MBHBCARMA::computeACVF(int numLags, double *Lags, double* ACVF) {
	#ifdef DEBUG_COMPUTEACVF
	int threadNum = omp_get_thread_num();
	#endif
	complex<double> alpha = kali::complexOne, beta = kali::complexZero;
	complex<double> *expw_acvf = static_cast<complex<double>*>(_mm_malloc(pSq*sizeof(complex<double>), 64));
	double T = 0.0, *F_acvf = static_cast<double*>(_mm_malloc(pSq*sizeof(double), 64));

	for (int colCtr = 0; colCtr < p; ++colCtr) {
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			expw_acvf[rowCtr + p*colCtr] = kali::complexZero;
			F_acvf[rowCtr + p*colCtr] = 0.0;
			}
		}

	#ifdef DEBUG_COMPUTEACVF
	printf("computeACVF - threadNum: %d; Address of System: %p\n",threadNum,this);
	printf("\n");
	#endif

	for (int lagNum = 0; lagNum < numLags; ++lagNum) {

		T = Lags[lagNum];

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("T: %+8.7e", T);
		#endif

		#pragma omp simd
		for (int i = 0; i < p; ++i) {
			expw_acvf[i + i*p] = exp(T*w[i]);
			}

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; expw_acvf = exp(w*T)\n",threadNum);
		viewMatrix(p,p,expw_acvf);
		printf("\n");
		#endif

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; vr (Before)\n",threadNum);
		viewMatrix(p,p,vr);
		printf("\n");
		printf("computeACVF - threadNum: %d; expw_acvf (Before)\n",threadNum);
		viewMatrix(p,p,expw);
		printf("\n");
		printf("computeACVF - threadNum: %d; ACopy = vr*expw_acvf (Before)\n",threadNum);
		viewMatrix(p,p,ACopy);
		printf("\n");
		#endif

		cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, vr, p, expw_acvf, p, &beta, ACopy, p);

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; vr (After)\n",threadNum);
		viewMatrix(p,p,vr);
		printf("\n");
		printf("computeACVF - threadNum: %d; expw_acvf (After)\n",threadNum);
		viewMatrix(p,p,expw_acvf);
		printf("\n");
		printf("computeACVF - threadNum: %d; ACopy = vr*expw_acvf (After)\n",threadNum);
		viewMatrix(p,p,ACopy);
		printf("\n");
		#endif

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; ACopy (Before)\n",threadNum);
		viewMatrix(p,p,ACopy);
		printf("\n");
		printf("computeACVF - threadNum: %d; vrInv (Before)\n",threadNum);
		viewMatrix(p,p,vrInv);
		printf("\n");
		printf("computeACVF - threadNum: %d; AScratch = ACopy*vrInv (Before)\n",threadNum);
		viewMatrix(p,p,AScratch);
		printf("\n");
		#endif

		cblas_zgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, &alpha, ACopy, p, vrInv, p, &beta, AScratch, p);

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; ACopy (After)\n",threadNum);
		viewMatrix(p,p,ACopy);
		printf("\n");
		printf("computeACVF - threadNum: %d; vrInv (After)\n",threadNum);
		viewMatrix(p,p,vrInv);
		printf("\n");
		printf("computeACVF - threadNum: %d; AScratch = aCopy*vrInv (After)\n",threadNum);
		viewMatrix(p,p,AScratch);
		printf("\n");
		#endif

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; AScratch (Before)\n",threadNum);
		viewMatrix(p,p,AScratch);
		printf("\n");
		printf("computeACVF - threadNum: %d; F_acvf = AScratch.real() (Before)\n",threadNum);
		viewMatrix(p,p,F_acvf);
		printf("\n");
		#endif

		for (int colCtr = 0; colCtr < p; ++colCtr) {
			#pragma omp simd
			for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
				F_acvf[rowCtr + colCtr*p] = AScratch[rowCtr + colCtr*p].real();
				}
			}

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; AScratch (After)\n",threadNum);
		viewMatrix(p,p,AScratch);
		printf("\n");
		printf("computeACVF - threadNum: %d; F_acvf = AScratch.real() (After)\n",threadNum);
		viewMatrix(p,p,F_acvf);
		printf("\n");
		#endif

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; F_acvf (Before)\n",threadNum);
		viewMatrix(p,p,F_acvf);
		printf("\n");
		printf("computeACVF - threadNum: %d; Sigma (Before)\n",threadNum);
		viewMatrix(p,p,Sigma);
		printf("\n");
		printf("computeACVF - threadNum: %d; MScratch = F_acvf*Sigma (Before)\n",threadNum);
		viewMatrix(p,p,MScratch);
		printf("\n");
		#endif

		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F_acvf, p, Sigma, p, 0.0, MScratch, p);

		#ifdef DEBUG_COMPUTEACVF
		printf("computeACVF - threadNum: %d; walkerPos: ",threadNum);
		for (int dimNum = 0; dimNum < kali::MBHBCARMA::r+p+q+1; dimNum++) {
			printf("%+8.7e ",Theta[dimNum]);
			}
		printf("\n");
		printf("computeACVF - threadNum: %d; F_acvf (After)\n",threadNum);
		viewMatrix(p,p,F_acvf);
		printf("\n");
		printf("computeACVF - threadNum: %d; Sigma (After)\n",threadNum);
		viewMatrix(p,p,Sigma);
		printf("\n");
		printf("computeACVF - threadNum: %d; MScratch = F_acvf*Sigma (After)\n",threadNum);
		viewMatrix(p,p,MScratch);
		printf("\n");
		#endif

		ACVF[lagNum] = MScratch[0];
		}
	if (expw_acvf) {
		_mm_free(expw_acvf);
		expw_acvf = nullptr;
		}
	if (F_acvf) {
		_mm_free(F_acvf);
		F_acvf = nullptr;
		}
	}*/

int kali::MBHBCARMA::RTSSmoother(LnLikeData *ptr2Data, double *XSmooth, double *PSmooth, double *xSmooth, double *xerrSmooth) {
	kali::LnLikeData Data = *ptr2Data;

	int numCadences = Data.numCadences;
	double tolIR = Data.tolIR;
	double *t = Data.t;
	double *y = Data.y;
	double *yerr = Data.yerr;
	double *mask = Data.mask;
    double startT = Data.startT*kali::Day;
	double maxDouble = numeric_limits<double>::max();

	mkl_domain_set_num_threads(1, MKL_DOMAIN_ALL);
	double t_incr = 0.0, LnLikelihood = 0.0, ptCounter = 0.0, v = 0.0, S = 0.0, SInv = 0.0, fracChange = 0.0, Contrib = 0.0;
	lapack_int YesNo;

	// Allocate arrays to hold KScratch, XMinus, PMinus, X, and P for the backward recursion
	double *KScratch, *XMinusList = nullptr, *PMinusList = nullptr, *XList = nullptr, *PList = nullptr;
	KScratch = static_cast<double*>(_mm_malloc(pSq*sizeof(double),64));
	XMinusList = static_cast<double*>(_mm_malloc(p*numCadences*sizeof(double),64));
	PMinusList = static_cast<double*>(_mm_malloc(pSq*numCadences*sizeof(double),64));
	XList = static_cast<double*>(_mm_malloc(p*numCadences*sizeof(double),64));
	PList = static_cast<double*>(_mm_malloc(pSq*numCadences*sizeof(double),64));

	// Forward iteration of RTS Smoother
	// Iterate through first point
    setEpoch(t[0]);
	H[0] = mask[0];
	R[0] = yerr[0]*yerr[0]; // Heteroskedastic errors
	cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
	cblas_dcopy(p, XMinus, 1, &XMinusList[0], 1); // Copy XMinus into XMinusList[0:p]
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;
	cblas_dcopy(pSq, PMinus, 1, &PMinusList[0], 1); // Copy PMinus into PMinusList[0:pSq]
    double yVal = (y[0]/getBeamingFactor2()) - totalFlux;
	v = mask[0]*(yVal - H[0]*XMinus[0]); // Compute v = y - H*X
	cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
	S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R
	SInv = 1.0/S;
	cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
	for (int colCounter = 0; colCounter < p; colCounter++) {
		#pragma omp simd
		for (int rowCounter = 0; rowCounter < p; rowCounter++) {
			MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
			}
		}
	cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
	cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, yVal, VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
	cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
	cblas_dcopy(p, X, 1, &XList[0], 1); // Copy X into XList[0:p]
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
	cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
	for (int colCounter = 0; colCounter < p; colCounter++) {
		#pragma omp simd
		for (int rowCounter = 0; rowCounter < p; rowCounter++) {
			P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
			}
		}
	cblas_dcopy(pSq, P, 1, &PList[0], 1); // Copy P into PList[0:pSq]
	Contrib = mask[0]*(-0.5*SInv*pow(v,2.0) -0.5*log2(S)/kali::log2OfE);
	LnLikelihood = LnLikelihood + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
	ptCounter = ptCounter + 1*static_cast<int>(mask[0]);
	// Iterate through remaining points
	for (int i = 1; i < numCadences; i++) {
        setEpoch(t[i]);
		t_incr = t[i] - t[i - 1];
		fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));
		if (fracChange > tolIR) {
			dt = t_incr;
			solveMBHBCARMA();
			}
		H[0] = mask[i];
		R[0] = yerr[i]*yerr[i]; // Heteroskedastic errors
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, F, p, X, 1, 0.0, XMinus, 1); // Compute XMinus = F*X
		cblas_dcopy(p, XMinus, 1, &XMinusList[i*p], 1); // Copy XMinus into XMinusList[i*p:(i+1)*p]
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, F, p, P, p, 0.0, MScratch, p); // Compute MScratch = F*P
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, F, p, 0.0, PMinus, p); // Compute PMinus = MScratch*F_Transpose
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, I, p, Q, p, 1.0, PMinus, p); // Compute PMinus = I*Q + PMinus;
		#ifdef DEBUG_RTSSMOOTHER
			printf("PMinus_{%d}\n",i);
			viewMatrix(p,p,PMinus);
		#endif
		cblas_dcopy(pSq, PMinus, 1, &PMinusList[i*pSq], 1); // Copy PMinus into PMinusList[i*pSq:(i+1)*pSq]
        yVal = (y[i]/getBeamingFactor2()) - totalFlux;
    	v = mask[i]*(yVal - H[0]*XMinus[0]); // Compute v = y - H*X
		cblas_dgemv(CblasColMajor, CblasTrans, p, p, 1.0, PMinus, p, H, 1, 0.0, K, 1); // Compute K = PMinus*H_Transpose
		S = cblas_ddot(p, K, 1, H, 1) + R[0]; // Compute S = H*K + R
		SInv = 1.0/S;
		cblas_dscal(p, SInv, K, 1); // Compute K = SInv*K
		for (int colCounter = 0; colCounter < p; colCounter++) {
			#pragma omp simd
			for (int rowCounter = 0; rowCounter < p; rowCounter++) {
				MScratch[rowCounter*p+colCounter] = I[colCounter*p+rowCounter] - K[colCounter]*H[rowCounter]; // Compute MScratch = I - K*H
				}
			}
		cblas_dcopy(p, K, 1, VScratch, 1); // Compute VScratch = K
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, MScratch, p, XMinus, 1, yVal, VScratch, 1); // Compute X = VScratch*y[i] + MScratch*XMinus
		cblas_dcopy(p, VScratch, 1, X, 1); // Compute X = VScratch
		cblas_dcopy(p, X, 1, &XList[i*p], 1); // Copy X into XList[i*p:(i+1)*p]
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, MScratch, p, PMinus, p, 0.0, P, p); // Compute P = IMinusKH*PMinus
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, P, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = P*IMinusKH_Transpose
		for (int colCounter = 0; colCounter < p; colCounter++) {
			#pragma omp simd
			for (int rowCounter = 0; rowCounter < p; rowCounter++) {
				P[colCounter*p+rowCounter] = PMinus[colCounter*p+rowCounter] + R[0]*K[colCounter]*K[rowCounter]; // Compute P = PMinus + K*R*K_Transpose
				}
			}
		cblas_dcopy(pSq, P, 1, &PList[i*pSq], 1); // Copy P into PList[i*pSq:(i+1)*pSq]
		Contrib = mask[i]*(-0.5*SInv*pow(v,2.0) -0.5*log2(S)/kali::log2OfE);
		LnLikelihood = LnLikelihood + Contrib; // LnLike += -0.5*v*v*SInv -0.5*log(det(S)) -0.5*log(2.0*pi)
		ptCounter = ptCounter + 1*static_cast<int>(mask[0]);
		}
	LnLikelihood += -0.5*ptCounter*kali::log2Pi;

	#ifdef DEBUG_RTSSMOOTHER
		printf("\n");
		printf("P\n");
		viewMatrix(p,p,P);
		printf("PList_{%d}\n",numCadences-1);
		viewMatrix(p,p,&PList[(numCadences-1)*pSq]);
	#endif

	// Reverse iteration of RTS Smoother
	// Iterate through last point
	cblas_dcopy(p, &XList[(numCadences - 1)*p], 1, &XSmooth[(numCadences - 1)*p], 1); // Compute XSmooth_{N} = X^{+}_{N}
    xSmooth[numCadences - 1] = (XSmooth[(numCadences - 1)*p] + totalFlux)*getBeamingFactor2();
	cblas_dcopy(pSq, &PList[(numCadences - 1)*pSq], 1, &PSmooth[(numCadences - 1)*pSq], 1); // Compute PSmooth_{N} = P^{+}_{N}
    try {
        xerrSmooth[numCadences - 1] = sqrt(PSmooth[(numCadences - 1)*pSq])*getBeamingFactor2();
    } catch (exception &err) {
        xerrSmooth[numCadences - 1] = 0.0;
    }
	#ifdef DEBUG_RTSSMOOTHER
		printf("PSmooth_{%d}\n",numCadences-1);
		viewMatrix(p,p,&PSmooth[(numCadences - 1)*pSq]);
	#endif
	// Iterate backwards through remaining points
	for (int i = numCadences - 2; i > -1; --i) {
        setEpoch(t[i]);
        t_incr = t[i] - t[i - 1];
		fracChange = abs((t_incr - dt)/((t_incr + dt)/2.0));
		if (fracChange > tolIR) {
			dt = t_incr;
			solveMBHBCARMA();
			}

		// Compute K_{i} = P_{i}*F_Transpose*PMinus_{i + 1}^{-1}
		#ifdef DEBUG_RTSSMOOTHER
			printf("\n");
			printf("i:%d\n",i);
			printf("PMinusList_{%d}\n",i+1);
			viewMatrix(p,p,&PMinusList[(i + 1)*pSq]);
		#endif
		for (int colCtr = 0; colCtr < p; ++colCtr) {
			#pragma omp simd
			for (int rowCtr = colCtr; rowCtr < p; ++rowCtr) {
				PMinus[rowCtr + p*colCtr] = PMinusList[(i + 1)*pSq + rowCtr + p*colCtr]; // Copy the lower-triangle of PMinus_{i + 1} into PMinus
				}
			#pragma omp simd
			for (int rowCtr = 0; rowCtr < colCtr; ++rowCtr) {
				PMinus[rowCtr + p*colCtr] = 0.0; // Copy the lower-triangle of PMinus_{i + 1} into PMinus
				}
			}
		#ifdef DEBUG_RTSSMOOTHER
			printf("PMinus\n,");
			viewMatrix(p,p,PMinus);
		#endif
		YesNo = LAPACKE_dpotrf(LAPACK_COL_MAJOR, 'L', p, PMinus, p); // Factor PMinus
		YesNo = LAPACKE_dpotri(LAPACK_COL_MAJOR, 'L', p, PMinus, p); // Compute Inverse of factorized PMinus
		for (int colCtr = 0; colCtr < p; ++colCtr) {
			#pragma omp simd
			for (int rowCtr = colCtr; rowCtr < p; ++rowCtr) {
				MScratch[rowCtr + p*colCtr] = PMinus[rowCtr + p*colCtr]; // Copy the PMinus_Inverse into MScratch
				MScratch[colCtr + p*rowCtr] = PMinus[rowCtr + p*colCtr]; // Copy the PMinus_Inverse into MScratch
				}
			}
		#ifdef DEBUG_RTSSMOOTHER
			printf("MScratch = PMinus_Inverse\n");
			viewMatrix(p,p,MScratch);
			printf("F (before)\n");
			viewMatrix(p,p,F);
			printf("MScratch (before)\n");
			viewMatrix(p,p,MScratch);
			printf("PMinus (before)\n");
			viewMatrix(p,p,PMinus);
		#endif
		cblas_dgemm(CblasColMajor, CblasTrans, CblasNoTrans, p, p, p, 1.0, F, p, MScratch, p, 0.0, PMinus, p); // Compute PMinus = F_Transpose*MScratch
		#ifdef DEBUG_RTSSMOOTHER
			printf("F (after)\n");
			viewMatrix(p,p,F);
			printf("MScratch (after)\n");
			viewMatrix(p,p,MScratch);
			printf("PMinus (after)\n");
			viewMatrix(p,p,PMinus);
			printf("PList_{%d} (before)\n",i);
			viewMatrix(p,p,&PList[i*pSq]);
			printf("PMinus (before)\n");
			viewMatrix(p,p,PMinus);
			printf("KScratch (before)\n");
			viewMatrix(p,p,KScratch);
		#endif
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, &PList[i*pSq], p, PMinus, p, 0.0, KScratch, p); // Compute KScratch = P_{i}*PMinus
		#ifdef DEBUG_RTSSMOOTHER
			printf("PList_{%d} (after)\n",i);
			viewMatrix(p,p,&PList[i*pSq]);
			printf("PMinus (after)\n");
			viewMatrix(p,p,PMinus);
			printf("KScratch (after)\n");
			viewMatrix(p,p,KScratch);
		#endif

		// Compute PSmooth_{i} = P_{i} + K_{i}*(PSmooth_{i + 1} - PMinus_{i + 1})*K_Transpose_{i}
		for (int colCtr = 0; colCtr < p; ++colCtr) {
			#pragma omp simd
			for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
				PMinus[rowCtr + p*colCtr] = PSmooth[(i + 1)*pSq + rowCtr + p*colCtr] - PMinusList[(i + 1)*pSq + rowCtr + p*colCtr]; // Compute PMinus = PSmooth_{i + 1} - PMinus_{i + 1}
				}
			}
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasNoTrans, p, p, p, 1.0, KScratch, p, PMinus, p, 0.0, MScratch, p); // Compute MScratch = KScratch*PMinus
		cblas_dcopy(pSq, &PList[i*pSq], 1, PMinus, 1); // Copy PMinus = PSmooth_{i}
		cblas_dgemm(CblasColMajor, CblasNoTrans, CblasTrans, p, p, p, 1.0, MScratch, p, KScratch, p, 1.0, PMinus, p); // Compute PMinus = PMinus + MScratch*KScratch_Transpose
		cblas_dcopy(pSq, PMinus, 1, &PSmooth[i*pSq], 1); // Copy PSmooth_{i} = PMinus
        try {
            xerrSmooth[i] = sqrt(PSmooth[i*pSq])*getBeamingFactor2();
        } catch (exception &err) {
            xerrSmooth[i] = 0.0;
        }

		// Compute XSmooth_{i} = X_{i} + K_{i}*(XSmooth_{i + 1} - XMinus_{i + 1})
		#pragma omp simd
		for (int rowCtr = 0; rowCtr < p; ++rowCtr) {
			VScratch[rowCtr] = XSmooth[(i + 1)*p + rowCtr] - XMinusList[(i + 1)*p + rowCtr]; // Compute VScratch = XMinus_{i + 1} - XSmooth_{i + 1}
			}
		cblas_dcopy(p, &XList[i*p], 1, XMinus, 1); // Copy XMinus = XSmooth_{i}
		cblas_dgemv(CblasColMajor, CblasNoTrans, p, p, 1.0, KScratch, p, VScratch, 1, 1.0, XMinus, 1); // Compute XMinus = XMinus + KScratch*VScratch
		cblas_dcopy(p, XMinus, 1, &XSmooth[i*p], 1);// Copy XSmooth_{i} = VScratch
        xSmooth[i] = (XSmooth[i*p] + totalFlux)*getBeamingFactor2();
        }

	// Deallocate arrays used to hold MScratch2, XMinus, PMinus, X and P for backward recursion
	if (KScratch) {
		_mm_free(KScratch);
		KScratch = nullptr;
		}
	if (XMinusList) {
		_mm_free(XMinusList);
		XMinusList = nullptr;
		}
	if (PMinusList) {
		_mm_free(PMinusList);
		PMinusList = nullptr;
		}
	if (XList) {
		_mm_free(XList);
		XList = nullptr;
		}
	if (PList) {
		_mm_free(PList);
		PList = nullptr;
		}

	Data.cadenceNum = numCadences - 1;
	Data.currentLnLikelihood = LnLikelihood;
	return 0;
}

int kali::computeAux(int ndims, int nwalkers, int nsteps, double sigmaStars, double H, double rhoStars, double *Chain, double *auxillaryChain) {
    #pragma omp parallel for default(none) shared(ndims, nwalkers, nsteps, sigmaStars, H, rhoStars, Chain, auxillaryChain, kali::Parsec, kali::Day, kali::fourPiSq, kali::G, kali::SolarMass, kali::c, kali::kms2ms, kali::SolarMassPerCubicParsec, kali::Year)
    for (int stepNum = 0; stepNum < nsteps; ++stepNum) {
        double a1 = 0.0, a2 = 0.0, T = 0.0, eccentricity = 0.0;
        double MTot = 0.0, MRat = 0.0, m1 = 0.0, m2 = 0.0, MRed = 0.0, rPeri = 0.0, rApo = 0.0, rSch = 0.0, aHard = 0.0, numer = 0.0, denom = 0.0, aGW = 0.0, THard = 0.0, MEject = 0.0;
        for (int walkerNum = 0; walkerNum < nwalkers; ++walkerNum) {
            a1 = kali::Parsec*Chain[0 + walkerNum*ndims + stepNum*nwalkers*ndims];
            a2 = kali::Parsec*Chain[1 + walkerNum*ndims + stepNum*nwalkers*ndims];
            T = kali::Day*Chain[2 + walkerNum*ndims + stepNum*nwalkers*ndims];
            eccentricity = Chain[3 + walkerNum*ndims + stepNum*nwalkers*ndims];
            MTot = ((kali::fourPiSq*pow(a1 + a2, 3.0))/(kali::G*pow(T, 2.0)))/kali::SolarMass;
            MRat = a1/a2;
            m1 = (MTot*(1.0/(1.0 + MRat)))*kali::SolarMass;
            m2 = (MTot*(MRat/(1.0 + MRat)))*kali::SolarMass;
            MRed = m1*m2/(m1 + m2);
            rPeri = ((a1 + a2)*(1.0 - eccentricity))/kali::Parsec;
            rApo = ((a1 + a2)*(1.0 + eccentricity))/kali::Parsec;
            rSch = ((2.0*kali::G*(MTot*kali::SolarMass))/(pow(kali::c, 2.0)))/kali::Parsec;
            aHard = ((kali::G*MRed)/(4.0*pow((sigmaStars*kali::kms2ms), 2.0)))/kali::Parsec;
            numer = 64.0*pow(kali::G, 2.0)*m1*m2*(MTot*kali::SolarMass)*(kali::kms2ms*sigmaStars);
            denom = 5.0*H*pow(kali::c, 5.0)*(kali::SolarMassPerCubicParsec*rhoStars);
            aGW = pow(numer/denom, 0.2)/kali::Parsec;
            THard = ((sigmaStars*kali::kms2ms)/(H*kali::G*(kali::SolarMassPerCubicParsec*rhoStars)*aGW*kali::Parsec))/kali::Year;
            MEject = (MTot*log(aHard/aGW))/1.0e6;
            MTot = MTot/1.0e6;
            auxillaryChain[0 + walkerNum*13 + stepNum*nwalkers*13] = Chain[0 + walkerNum*ndims + stepNum*nwalkers*ndims];
            auxillaryChain[1 + walkerNum*13 + stepNum*nwalkers*13] = Chain[1 + walkerNum*ndims + stepNum*nwalkers*ndims];
            auxillaryChain[2 + walkerNum*13 + stepNum*nwalkers*13] = Chain[2 + walkerNum*ndims + stepNum*nwalkers*ndims];
            auxillaryChain[3 + walkerNum*13 + stepNum*nwalkers*13] = Chain[3 + walkerNum*ndims + stepNum*nwalkers*ndims];
            auxillaryChain[4 + walkerNum*13 + stepNum*nwalkers*13] = MTot;
            auxillaryChain[5 + walkerNum*13 + stepNum*nwalkers*13] = MRat;
            auxillaryChain[6 + walkerNum*13 + stepNum*nwalkers*13] = rPeri;
            auxillaryChain[7 + walkerNum*13 + stepNum*nwalkers*13] = rApo;
            auxillaryChain[8 + walkerNum*13 + stepNum*nwalkers*13] = rSch;
            auxillaryChain[9 + walkerNum*13 + stepNum*nwalkers*13] = aHard;
            auxillaryChain[10 + walkerNum*13 + stepNum*nwalkers*13] = aGW;
            auxillaryChain[11 + walkerNum*13 + stepNum*nwalkers*13] = THard;
            auxillaryChain[12 + walkerNum*13 + stepNum*nwalkers*13] = MEject;
        }
    }
    return 0;
}
