#include <iostream>
#include <cstdio>
#include <thread>
#include <vector>
#include <string>
#include <bitset>
#include <cstring>
#include <cmath>
#include <ctime>
#include <algorithm>
#include "fingerprint-extractor.h"
#include "util.h"
#include "type.h"

using namespace std;

const double freq_bind[] =
{ 300.000, 317.752, 336.554, 356.469, 377.563,
399.904, 423.568, 448.632, 475.178, 503.296,
533.078, 564.622, 598.032, 633.419, 670.901,
710.600, 752.648, 797.185, 844.357, 894.320,
947.240, 1003.29, 1062.66, 1125.54, 1192.14,
1262.68, 1337.40, 1416.54, 1500.36, 1589.14,
1683.17, 1782.77, 1888.27, 2000.00 }; //划分的频带 [0, 33]

FingerExtractor::~FingerExtractor()
{}

void FingerExtractor::CalcFingerprint(const string waveFilePath) {
	this->wavepath = waveFilePath;
	wp.Clear();
	wp.OpenWaveFile(waveFilePath.c_str());
	wp.MakeTargetSamplesData();
	unsigned long all_time_data_size = 0;
	wp.GetSamplesVector(all_time_data, all_time_data_size);
	wp.CloseWaveFile();
	_Energying(all_time_data_size);
	_Fingerprinting();
}

void FingerExtractor::_calc_freq_bind() {
	double a1 = 300;
	double a33 = 2000;
	double q = pow((double)2000 / 300, 1.0 / 33);
	double temp = a1;
	//freq_bind.push_back(temp);
	for (int i = 1; i <= 33; i++) {
		temp *= q;
		//freq_bind.push_back(temp);
	}

	/*
	for(int i = 0; i < 34; i++)
	cout<<"i:"<<i<<" :"<<freq_bind[i]<<endl;
	*/
}

int FingerExtractor::_select_bind(double point_freq) {
	int start = 0;
	int end = 33;
	int mid = 0;
	while (start <= end) {
		mid = (end + start) / 2;
		if (point_freq < freq_bind[mid])
			end = mid - 1;
		else if (point_freq > freq_bind[mid + 1])
			start = mid + 1;
		else
			return mid;
	}
	return -1;
}
/*
int FingerExtractor::_multi_thread_FFT(int thread_num, int frame_size, int frame_index[THREAD_NUM])
{
int real_frame_index = 0;
for(int frameNum = 0; frameNum < frame_size; frameNum++)
{
cpxv_t freq_data[2048];
double bind_energy[33];
memset(bind_energy, 0, sizeof(double) * 33);
DoFFT(time_data_per_frame[thread_num][frameNum], freq_data);
double point_freq = 0;
for(int j = 0; j < NumBinsInFftWinM; j++)
{
//FFT结果第n个点代表的频率值
point_freq = (j + 1) * sampleRate / NumBinsInFftWinM;
if(point_freq < 300 || point_freq > 2000)
continue;
else
{
int bind = _select_bind(point_freq); // [0,32]
bind_energy[bind] += sqrt((freq_data[j].re * freq_data[j].re + freq_data[j].im * freq_data[j].im));
}
}
real_frame_index = frame_index[frameNum];
for(int i = 0; i < 33; i++)
energy[real_frame_index][i] = bind_energy[i];
}
return 0;
}
*/
int FingerExtractor::_Energying(long all_time_data_size) {
	//clock_t FFT_start, FFT_end;

	memset(energy, 0, sizeof(double)* QUERY_FINGER_NUM * 33);
	frameNum = 0;
	int start = 0;
	int jump_samples = (int)(sampleRate * timeInterval);

	/*
	#ifdef MULTITHREAD_FFT
	memset(time_data_per_frame, 0, sizeof(short) * SUB_FINGER_NUM * NumSamplesPerFrameM);
	int temp_thread_num = 0;
	int temp_frame_num = 0;
	int frame_size[FFT_THREAD];
	int frame_index[FFT_THREAD][SUB_FINGER_NUM];
	memset(frame_size, 0, sizeof(int) * FFT_THREAD);
	while(start + NumSamplesPerFrameM < all_time_data_size)
	{
	temp_thread_num = frameNum % FFT_THREAD;
	temp_frame_num = frame_size[temp_thread_num];
	frame_index[temp_thread_num][temp_frame_num] = frameNum;
	for(int i = 0; i < NumSamplesPerFrameM; i++)
	{
	time_data_per_frame[temp_thread_num][temp_frame_num][i] = all_time_data[i + start];
	}
	frame_size[temp_thread_num]++;
	frameNum++;
	start += jump_samples;
	}
	// load data finished
	vector<thread> threads;
	for(int i = 0; i < FFT_THREAD; i++)
	{
	threads.push_back(thread(&FingerExtractor::_multi_thread_FFT, this, i, frame_size[i], frame_index[i]));
	}
	for(int i = 0; i < FFT_THREAD; i++)
	{
	threads[i].join();
	}

	#else
	*/
	while (start + NumSamplesPerFrameM < all_time_data_size) {
		short time_data[1850];
		cpxv_t freq_data[2048];
		double bind_energy[33];
		memset(bind_energy, 0, sizeof(double)* 33);
		for (int i = 0; i < NumSamplesPerFrameM; i++) {
			time_data[i] = all_time_data[i + start];
		}

		//FFT_start = clock();
		DoFFT(time_data, freq_data);
		//FFT_end = clock();
		//duration_FFT += (double)(FFT_end - FFT_start) / CLOCKS_PER_SEC;
		double point_freq = 0;
		for (int j = 0; j < NumBinsInFftWinM; j++) {
			//FFT结果第n个点代表的频率值
			point_freq = (j + 1) * sampleRate / NumBinsInFftWinM;
			if (point_freq < 300 || point_freq > 2000) {
				continue;
			} else {
				int bind = _select_bind(point_freq); // [0,32]
				bind_energy[bind] += sqrt((freq_data[j].re * freq_data[j].re + freq_data[j].im * freq_data[j].im));
			}
		}
		for (int i = 0; i < 33; i++)
			energy[frameNum][i] = bind_energy[i];

		//下一帧
		frameNum++;
		start += jump_samples;
	}

	//#endif
	return frameNum;
}

/*
vector<short> FingerExtractor::_get_top_N(vector<double>& energy)
{
struct _top_energy
{
double data;
short index;
};
vector<_top_energy> top_energy;
for(int i = 0; i < (signed)energy.size(); i++)
{
struct  _top_energy t;
t.data = energy[i];
t.index = i;
top_energy.push_back(t);
}
sort(top_energy.begin(), top_energy.end(), [](struct _top_energy t1, struct _top_energy t2){return abs(t1.data) < abs(t2.data);});

vector<short> top;
for(int i = 0; i < TOGGLENUM; i++)
top.push_back(top_energy[i].index);
return top;
}
*/

void FingerExtractor::_Fingerprinting() {
	memset(fingers_energy, 0, sizeof(double)* 204 * 32);
	//第0帧
	for (int j = 0; j < 32; j++) {
		fingers_energy[0][j] = energy[0][j] - energy[0][j + 1];
		if (fingers_energy[0][j] > 0)
			fingers[0][j] = '1';
		else
			fingers[0][j] = '0';
	}

	for (int i = 1; i < frameNum; i++) {
		for (int j = 0; j < 32; j++) {
			fingers_energy[i][j] = energy[i][j] - energy[i][j + 1] - (energy[i - 1][j] - energy[i - 1][j + 1]);
			if (fingers_energy[i][j] > 0)
				fingers[i][j] = '1';
			else
				fingers[i][j] = '0';
		}
	}
}

void FingerExtractor::getQueryFinger(FingerItem* new_finger, int& size) {
	size = frameNum;
	for (int i = 0; i < frameNum; i++) {
		FingerItem item;
		bitset<32> b(fingers[i]);
#ifdef OPEN_NEAR_SEARCH
		item.toggle_bits = _get_top_N(fingers_energy[i]);
#endif
		item.finger = b;
		new_finger[i] = item;
	}
	return;
}

int FingerExtractor::getFingerFileId() {
	string originFile = wavepath.substr(wavepath.find_last_of("\\") + 1, wavepath.size());
	return stoi(originFile.substr(0, originFile.find(".")));
}

int FingerExtractor::PrintFingerToFile(const string fingerFile) {
	FILE *fp = fopen(fingerFile.c_str(), "w");
	string sub_finger;
	for (int i = 0; i < frameNum; i++) {
		sub_finger = "";
		for (int j = 0; j < 32; j++)
			sub_finger.push_back(fingers[i][j]);
		bitset<32> b(sub_finger);
		fprintf(fp, "%lu\n", b.to_ulong());
	}
	fclose(fp);
	return 0;
}