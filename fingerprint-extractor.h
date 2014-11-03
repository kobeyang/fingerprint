#pragma once
#include <iostream>
#include <vector>
#include <bitset>
#include "FFT.h"
#include "wave-processor.h"
#include "Type.h"

extern const int NumBinsInFftWinM; // 2048
extern const int NumSamplesPerFrameM; //1850
const double timeInterval = 0.0232;
//const double timeInterval = 0.0116;
const double frameInterval = 0.37;
const int sampleRate = 5000;

extern double duration_FFT;

class FingerExtractor {
public:
	FingerExtractor() :wp(5000){};
	~FingerExtractor();
	void CalcFingerprint(const string waveFilePath);
	int PrintFingerToFile(const string filepath);
	void getQueryFinger(FingerItem* new_finger, int& size);
	int getFingerFileId();

	//vector<short> _get_top_N(vector<double>& energy);

private:
	void _calc_freq_bind();
	int _select_bind(double point_freq);
	int _multi_thread_FFT(int thread_num, int frame_size, int frame_index[FFT_THREAD]);
	string wavepath;
	int frameNum; //记录被处理的音频一共有多少帧

	short all_time_data[SamplesVectorSize]; // for query
	//short time_data_per_frame[FFT_THREAD][SUB_FINGER_NUM/FFT_THREAD][NumSamplesPerFrameM];
	double energy[SUB_FINGER_NUM][33]; //能量E[n,m]，表示第n帧的第m个频带的能量值
	double fingers_energy[SUB_FINGER_NUM][32]; //fingers[i][j] 的值 double类型
	char fingers[SUB_FINGER_NUM][32]; //最终的指纹结果

	WaveProcessor wp;

	int _Energying(long all_time_data_size);
	void _Fingerprinting();
};