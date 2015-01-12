#pragma once
#include <bitset>
#include <iostream>
#include <vector>
#include "fft.h"
#include "type.h"
#include "wave-processor.h"

extern const int NumBinsInFftWinM; // 2048
extern const int NumSamplesPerFrameM; //1850
const double frameInterval = 0.37;
const int sampleRate = 5000;

extern double duration_FFT;

class FingerExtractor {
public:
	FingerExtractor() :_wp(5000){};
	~FingerExtractor();
	void CalcFingerprint(const string waveFilePath);
	int PrintFingerToFile(const string filepath);
	void GetQueryFinger(FingerItem* new_finger, int& size);
	int GetFingerFileId();
	void OutputTrainingSamples(const std::string& x_path,
		const std::string& y_path);
	//vector<short> _get_top_N(vector<double>& energy);

private:
	void _CalcFreqBind();
	int _SelectBind(double point_freq);
	int _MultiThreadFFT(int thread_num, int frame_size, int frame_index[FFT_THREAD]);
	int _Energying(long all_time_data_size);
	void _Fingerprinting();

	WaveProcessor _wp;
	string _wavepath;
	int _frame_num; //记录被处理的音频一共有多少帧
	short _all_time_data[SamplesVectorSize]; // for query
	double _energy[SUB_FINGER_NUM][33]; //能量E[n,m]，表示第n帧的第m个频带的能量值
	char _fingers[SUB_FINGER_NUM][32]; //最终的指纹结果
	std::vector<std::vector<double>> _log_power; // The second demension is 2048.
};