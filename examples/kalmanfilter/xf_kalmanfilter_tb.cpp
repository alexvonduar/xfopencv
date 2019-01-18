/***************************************************************************
Copyright (c) 2018, Xilinx, Inc.
All rights reserved.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.
3. Neither the name of the copyright holder nor the names of its contributors
may be used to endorse or promote products derived from this software
without specific prior written permission.
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#include "xf_headers.h"
#include "xf_kalmanfilter_config.h"

void error_check(cv::KalmanFilter kf, float *Xout_ptr, float *Uout_ptr, float *Dout_ptr, bool tu_or_mu, float *error_out)
{

	ap_uint<32> nan_xf = 0x7fc00000;

	cv::Mat Uout(KF_N, KF_N, CV_32FC1,  Uout_ptr);
	cv::Mat Dout = cv::Mat::zeros(KF_N, KF_N, CV_32FC1);
	for(int i=0; i<KF_N; i++)
		Dout.at<float>(i,i) = Dout_ptr[i];

	cv::Mat Pout(KF_N, KF_N, CV_32FC1);
	Pout = ( (Uout * Dout) * Uout.t() );

	float tu_max_error_P=-10000;
	float tu_max_error_P_reference;
	float tu_max_error_P_kernel;
	int tu_max_error_P_index[2];
	for(int i=0; i<KF_N;i++)
	{
		for(int j=0; j<KF_N;j++)
		{
			float kernel_output = Pout.at<float>(i,j);
			float refernce_output;
			if(tu_or_mu==0)
				refernce_output = (float)kf.errorCovPre.at<double>(i,j);
			else
				refernce_output = (float)kf.errorCovPost.at<double>(i,j);

			float error = fabs(kernel_output - refernce_output);

			ap_uint<32> error_int = *(int*)(float*)&error;

			if(error >  tu_max_error_P || error_int==nan_xf){
				tu_max_error_P = error;
				tu_max_error_P_reference = refernce_output;
				tu_max_error_P_kernel = kernel_output;
				tu_max_error_P_index[0]=i;
				tu_max_error_P_index[1]=j;
			}

		}
	}

	float tu_max_error_X=-10000;
	float tu_max_error_X_reference;
	float tu_max_error_X_kernel;
	int tu_max_error_X_index;
	for(int i=0; i<KF_N;i++)
	{
		float kernel_output = Xout_ptr[i];

		float refernce_output;
		if(tu_or_mu==0)
			refernce_output = (float)kf.statePre.at<double>(i);
		else
			refernce_output = (float)kf.statePost.at<double>(i);

		float error = fabs(kernel_output - refernce_output);

		ap_uint<32> error_int = *(int*)(float*)&error;

		if(error >  tu_max_error_X || error_int==nan_xf){
			tu_max_error_X = error;
			tu_max_error_X_reference = refernce_output;
			tu_max_error_X_kernel = kernel_output;
			tu_max_error_X_index=i;
		}

	}

	if(tu_max_error_X > tu_max_error_P)
		*error_out = tu_max_error_X;
	else
		*error_out = tu_max_error_P;
}

int main(int argc, char *argv[])
{


#if __SDSCC__
	//#if 1
	fprintf(stderr,"\n*** SDS memory for Arguments");
	float *A_ptr 	= (float *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(float));
	float *B_ptr 	= (float *)sds_alloc_non_cacheable(KF_N*KF_C*sizeof(float));
	float *Uq_ptr 	= (float *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(float));
	float *Dq_ptr 	= (float *)sds_alloc_non_cacheable(KF_N*sizeof(float));
	float *H_ptr 	= (float *)sds_alloc_non_cacheable(KF_M*KF_N*sizeof(float));
	float *X0_ptr 	= (float *)sds_alloc_non_cacheable(KF_N*sizeof(float));
	float *U0_ptr 	= (float *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(float));
	float *D0_ptr 	= (float *)sds_alloc_non_cacheable(KF_N*sizeof(float));
	float *R_ptr 	= (float *)sds_alloc_non_cacheable(KF_M*sizeof(float));
	float *u_ptr 	= (float *)sds_alloc_non_cacheable(KF_C*sizeof(float));
	float *y_ptr 	= (float *)sds_alloc_non_cacheable(KF_M*sizeof(float));
	float *Xout_ptr = (float *)sds_alloc_non_cacheable(KF_N*sizeof(float));
	float *Uout_ptr = (float *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(float));
	float *Dout_ptr = (float *)sds_alloc_non_cacheable(KF_N*sizeof(float));

	double *A_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(double));
	double *B_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*KF_C*sizeof(double));
	double *Q_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(double));
	double *Uq_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(double));
	double *Dq_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*sizeof(double));
	double *H_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_M*KF_N*sizeof(double));
	double *X0_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*sizeof(double));
	double *P0_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(double));
	double *U0_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*KF_N*sizeof(double));
	double *D0_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_N*sizeof(double));
	double *R_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_M*sizeof(double));
	double *u_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_C*sizeof(double));
	double *y_ptr_dp 	= (double *)sds_alloc_non_cacheable(KF_M*sizeof(double));


#else

	float *A_ptr 	= (float *)malloc(KF_N*KF_N*sizeof(float));
	float *B_ptr 	= (float *)malloc(KF_N*KF_C*sizeof(float));
	float *Q_ptr 	= (float *)malloc(KF_N*KF_N*sizeof(float));
	float *Uq_ptr 	= (float *)malloc(KF_N*KF_N*sizeof(float));
	float *Dq_ptr 	= (float *)malloc(KF_N*sizeof(float));
	float *H_ptr 	= (float *)malloc(KF_M*KF_N*sizeof(float));
	float *X0_ptr 	= (float *)malloc(KF_N*sizeof(float));
	float *P0_ptr 	= (float *)malloc(KF_N*KF_N*sizeof(float));
	float *U0_ptr 	= (float *)malloc(KF_N*KF_N*sizeof(float));
	float *D0_ptr 	= (float *)malloc(KF_N*sizeof(float));
	float *R_ptr 	= (float *)malloc(KF_M*sizeof(float));
	float *u_ptr 	= (float *)malloc(KF_C*sizeof(float));
	float *y_ptr 	= (float *)malloc(KF_M*sizeof(float));
	float *Xout_ptr = (float *)malloc(KF_N*sizeof(float));
	float *Uout_ptr = (float *)malloc(KF_N*KF_N*sizeof(float));
	float *Dout_ptr = (float *)malloc(KF_N*sizeof(float));

	double *A_ptr_dp 	= (double *)malloc(KF_N*KF_N*sizeof(double));
	double *B_ptr_dp 	= (double *)malloc(KF_N*KF_C*sizeof(double));
	double *Q_ptr_dp 	= (double *)malloc(KF_N*KF_N*sizeof(double));
	double *Uq_ptr_dp 	= (double *)malloc(KF_N*KF_N*sizeof(double));
	double *Dq_ptr_dp 	= (double *)malloc(KF_N*sizeof(double));
	double *H_ptr_dp 	= (double *)malloc(KF_M*KF_N*sizeof(double));
	double *X0_ptr_dp 	= (double *)malloc(KF_N*sizeof(double));
	double *P0_ptr_dp 	= (double *)malloc(KF_N*KF_N*sizeof(double));
	double *U0_ptr_dp 	= (double *)malloc(KF_N*KF_N*sizeof(double));
	double *D0_ptr_dp 	= (double *)malloc(KF_N*sizeof(double));
	double *R_ptr_dp 	= (double *)malloc(KF_M*sizeof(double));
	double *u_ptr_dp 	= (double *)malloc(KF_C*sizeof(double));
	double *y_ptr_dp 	= (double *)malloc(KF_M*sizeof(double));

#endif

	//fprintf(stderr,"\n------ Init A");
	int Acnt=0;
	for(int i=0; i<KF_N;i++){
		for(int j=0; j<KF_N;j++){
			double val = ((double)rand()/(double)(RAND_MAX)) * 2.0;
			A_ptr_dp[Acnt] = val;
			A_ptr[Acnt++] = (float)val;
		}
	}

	//fprintf(stderr,"\n------ Init B");
	int Bcnt=0;
	for(int i=0; i<KF_N;i++){
		for(int j=0; j<KF_C;j++){
			double val = ((double)rand()/(double)(RAND_MAX)) * 1.0;
			B_ptr_dp[Bcnt] = val;
			B_ptr[Bcnt++] = (float)val;
		}
	}

	//fprintf(stderr,"\n------ Init H");
	int Hcnt=0;
	for(int i=0; i<KF_M;i++){
		for(int j=0; j<KF_N;j++){
			double val = ((double)rand()/(double)(RAND_MAX)) * 0.001;
			H_ptr_dp[Hcnt] = val;
			H_ptr[Hcnt++] = (float)val;
		}
	}

	//fprintf(stderr,"\n------ Init X0");
	for(int i=0; i<KF_N;i++){
		double val = ((double)rand()/(double)(RAND_MAX)) * 5.0;
		X0_ptr_dp[i] = val;
		X0_ptr[i] = (float)val;
	}

	//fprintf(stderr,"\n------ Init R");
	for(int i=0; i<KF_M;i++){
		double val = ((double)rand()/(double)(RAND_MAX)) * 0.01;
		R_ptr_dp[i] = val;
		R_ptr[i] = (float)val;
	}
	//fprintf(stderr,"\n------ Init U0");
	for(int i=0; i<KF_N;i++)
	{
		for(int jn=(-i), j = 0; j<KF_N; jn++, j++)
		{
			int index = j + i*KF_N;
			if(jn<0)
			{
				U0_ptr_dp[index] = 0;
				U0_ptr[index] = 0;
			}else if(jn==0){
				U0_ptr_dp[index] = 1;
				U0_ptr[index] = 1;
			}
			else
			{
				double val = ((double)rand()/(double)(RAND_MAX)) * 1.0;
				U0_ptr_dp[index] = val;
				U0_ptr[index] = (float)val;
			}
		}
	}

	//fprintf(stderr,"\n------ Init D0");
	for(int i=0; i<KF_N;i++)
	{
		double val = ((double)rand()/(double)(RAND_MAX)) * 1.0;
		D0_ptr_dp[i] = val;
		D0_ptr[i] = (float)val;
	}

	//fprintf(stderr,"\n------ Init Uq");
	for(int i=0; i<KF_N;i++)
	{
		for(int jn=(-i), j = 0; j<KF_N; jn++, j++)
		{
			int index = j + i*KF_N;
			if(jn<0)
			{
				Uq_ptr_dp[index] = 0;
				Uq_ptr[index] = 0;
			}else if(jn==0){
				Uq_ptr_dp[index] = 1;
				Uq_ptr[index] = 1;
			}
			else
			{
				double val = ((double)rand()/(double)(RAND_MAX)) * 1.0;
				Uq_ptr_dp[index] = val;
				Uq_ptr[index] = (float)val;
			}
		}
	}

	//fprintf(stderr,"\n------ Init Dq");
	for(int i=0; i<KF_N;i++)
	{
		double val = ((double)rand()/(double)(RAND_MAX)) * 1.0;
		Dq_ptr_dp[i] = val;
		Dq_ptr[i] = (float)val;
	}

	cv::Mat A(KF_N, KF_N, CV_64FC1,  A_ptr_dp);
	cv::Mat B(KF_N, KF_C, CV_64FC1,  B_ptr_dp);

	cv::Mat Uq(KF_N, KF_N,CV_64FC1,  Uq_ptr_dp);
	cv::Mat Dq = cv::Mat::zeros(KF_N, KF_N, CV_64FC1);
	for(int i=0; i<KF_N; i++)
		Dq.at<double>(i,i) = Dq_ptr_dp[i];
	cv::Mat Q(KF_N, KF_N,CV_64FC1);
	Q = Uq * Dq * Uq.t();

	cv::Mat H(KF_M, KF_N, CV_64FC1,  H_ptr_dp);
	cv::Mat X0(KF_N, 1,   CV_64FC1);
	for(int i=0; i<KF_N; i++)
		X0.at<double>(i) = X0_ptr_dp[i];

	cv::Mat U0(KF_N, KF_N,CV_64FC1,  U0_ptr_dp);
	cv::Mat D0 = cv::Mat::zeros(KF_N, KF_N, CV_64FC1);
	for(int i=0; i<KF_N; i++)
		D0.at<double>(i,i) = D0_ptr_dp[i];
	cv::Mat P0(KF_N, KF_N,CV_64FC1);
	P0 = U0 * D0 * U0.t();

	cv::Mat R = cv::Mat::zeros(KF_M, KF_M, CV_64FC1);
	for(int i=0; i<KF_M; i++)
		R.at<double>(i,i) = R_ptr_dp[i];
	cv::Mat uk(KF_C, 1,    CV_64FC1);
	cv::Mat zk(KF_M ,1,    CV_64FC1);

	xf::Mat<KF_TYPE, KF_N, KF_N, KF_NPC> 	A_mat;
	xf::Mat<KF_TYPE, KF_N, KF_C, KF_NPC> 	B_mat;
	xf::Mat<KF_TYPE, KF_N, KF_N, KF_NPC> 	Uq_mat;
	xf::Mat<KF_TYPE, KF_N, 1, KF_NPC> 		Dq_mat;
	xf::Mat<KF_TYPE, KF_M, KF_N, KF_NPC> 	H_mat;
	xf::Mat<KF_TYPE, KF_N, 1, KF_NPC> 		X0_mat;
	xf::Mat<KF_TYPE, KF_N, KF_N, KF_NPC> 	U0_mat;
	xf::Mat<KF_TYPE, KF_N, 1, KF_NPC> 		D0_mat;
	xf::Mat<KF_TYPE, KF_M, 1, KF_NPC> 		R_mat;
	xf::Mat<KF_TYPE, KF_C, 1, KF_NPC> 		u_mat;
	xf::Mat<KF_TYPE, KF_M, 1, KF_NPC> 		y_mat;
	xf::Mat<KF_TYPE, KF_N, 1, KF_NPC> 		Xout_mat;
	xf::Mat<KF_TYPE, KF_N, KF_N, KF_NPC> 	Uout_mat;
	xf::Mat<KF_TYPE, KF_N, 1, KF_NPC> 		Dout_mat;

	A_mat.copyTo(A_ptr);
	B_mat.copyTo(B_ptr);
	Uq_mat.copyTo(Uq_ptr);
	Dq_mat.copyTo(Dq_ptr);
	H_mat.copyTo(H_ptr);
	X0_mat.copyTo(X0_ptr);
	U0_mat.copyTo(U0_ptr);
	D0_mat.copyTo(D0_ptr);
	R_mat.copyTo(R_ptr);
	u_mat.copyTo(u_ptr);
	y_mat.copyTo(y_ptr);

	fprintf(stderr,"\n Kalman Filter Verification");
	fprintf(stderr,"\n Number of state variables: %d", KF_N);
	fprintf(stderr,"\n Number of measurements: %d", KF_M);
	fprintf(stderr,"\n Number of control input: %d\n", KF_C);

	//OpenCv Kalman Filter in Double Precision
	cv::KalmanFilter kf(KF_N, KF_M, KF_C, CV_64F);
	kf.statePost = X0;
	kf.errorCovPost = P0;
	kf.transitionMatrix = A;
	kf.processNoiseCov = Q;
	kf.measurementMatrix = H;
	kf.measurementNoiseCov =R;
	kf.controlMatrix = B;

	//fprintf(stderr,"\n------ Init control parameter");
	for(int i=0; i<KF_C;i++){
		double val = ((double)rand()/(double)(RAND_MAX)) * 10.0;
		u_ptr[i] = (float)val;
		uk.at<double>(i) = val;
	}
	u_mat.copyTo(u_ptr);

	//OpenCv Kalman Filter in Double Precision
	kf.predict(uk);

	//fprintf(stderr,"\n------ Init measurement parameter");
	for(int i=0; i<KF_M;i++){
		double val = ((double)rand()/(double)(RAND_MAX)) * 5.0;
		y_ptr[i] = (float)val;
		zk.at<double>(i) = val;
	}
	y_mat.copyTo(y_ptr);

	//OpenCv Kalman Filter in Double Precision
	kf.correct(zk);

	#if __SDSCC__
	perf_counter hw_ctr;
	hw_ctr.start();
	#endif

	//Xilinx Kalman filter in Single Precision
	kalmanfilter_accel(A_mat, B_mat, Uq_mat, Dq_mat,  H_mat, X0_mat, U0_mat, D0_mat, R_mat, u_mat, y_mat, Xout_mat, Uout_mat, Dout_mat, 103);

	Xout_ptr = (float *)Xout_mat.copyFrom();
	Uout_ptr = (float *)Uout_mat.copyFrom();
	Dout_ptr = (float *)Dout_mat.copyFrom();

	#if __SDSCC__
	hw_ctr.stop();
	uint64_t hw_cycles = hw_ctr.avg_cpu_cycles();
	#endif

	float error;
	error_check(kf, Xout_ptr, Uout_ptr, Dout_ptr, 1, &error);

	if(error < 0.001f)
	{
		fprintf(stderr,"\n********** Test Pass\n");
		return 0;
	}
	else
	{
		fprintf(stderr,"\n********** Test Fail\n");
		return -1;
	}
}
