#include <iostream>
#include <iomanip>
#include <fstream>
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

FingerExtractor::~FingerExtractor() {}

void FingerExtractor::CalcFingerprint(const string waveFilePath) {
	this->_wavepath = waveFilePath;
	_wp.Clear();
	_wp.OpenWaveFile(waveFilePath.c_str());
	_wp.MakeTargetSamplesData();
	unsigned long all_time_data_size = 0;
	_wp.GetSamplesVector(_all_time_data, all_time_data_size);
	_wp.CloseWaveFile();
	_Energying(all_time_data_size);
	_Fingerprinting();
}

int FingerExtractor::_SelectBind(double point_freq) {
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

int FingerExtractor::_Energying(long all_time_data_size) {
	//clock_t FFT_start, FFT_end;
	memset(_energy, 0, sizeof(double)* QUERY_FINGER_NUM * 33);
	_frame_num = 0;
	int start = 0;
	int jump_samples = (int)(sampleRate * TIME_INTERVAL);
	while (start + NumSamplesPerFrameM < all_time_data_size) {
		short time_data[1850];
		cpxv_t freq_data[2048];
		double bind_energy[33];
		memset(bind_energy, 0, sizeof(double)* 33);
		for (int i = 0; i < NumSamplesPerFrameM; i++) {
			time_data[i] = _all_time_data[i + start];
		}

		//FFT_start = clock();
		DoFFT(time_data, freq_data);
		//FFT_end = clock();
		//duration_FFT += (double)(FFT_end - FFT_start) / CLOCKS_PER_SEC;
		double point_freq = 0;
		_log_power.push_back(vector<double>(NumBinsInFftWinM));
		for (int i = 0; i < NumBinsInFftWinM; i++) {
			double power = (freq_data[i].re * freq_data[i].re + freq_data[i].im * freq_data[i].im);
			_log_power[_frame_num][i] = max(0.0, log(power));

			//FFT结果第n个点代表的频率值
			point_freq = (i + 1) * sampleRate / NumBinsInFftWinM;
			if (point_freq < 300 || point_freq > 2000) {
				continue;
			} else {
				int bind = _SelectBind(point_freq); // [0,32]
				bind_energy[bind] += sqrt(power);
			}
		}
		for (int i = 0; i < 33; i++)
			_energy[_frame_num][i] = bind_energy[i];

		//下一帧
		_frame_num++;
		start += jump_samples;
	}

	//#endif
	return _frame_num;
}

void FingerExtractor::_Fingerprinting() {
	//第0帧
	for (int j = 0; j < 32; j++) {
		if (_energy[0][j] - _energy[0][j + 1] > 0)
			_fingers[0][j] = '1';
		else
			_fingers[0][j] = '0';
	}

	for (int i = 1; i < _frame_num; i++) {
		for (int j = 0; j < 32; j++) {
			if (_energy[i][j] - _energy[i][j + 1] - (_energy[i - 1][j] - _energy[i - 1][j + 1]) > 0)
				_fingers[i][j] = '1';
			else
				_fingers[i][j] = '0';
		}
	}
}

void FingerExtractor::OutputTrainingSamples(const string& x_path,
	const string& y_path) {
	fstream fout_x, fout_y;
	fout_x.open(x_path, fstream::app);
	fout_y.open(y_path, fstream::app);
	int batch_size = 100;
	int jump = 10;
	int output_samples = _frame_num / (batch_size * jump) * batch_size * jump;
	for (int i = 0; i < output_samples; i += jump) {
		for (int j = 0; j < NumBinsInFftWinM; j++) {
			fout_x << setprecision(2) << std::fixed << _log_power[i][j] << "\t";
		}
		fout_x << endl;
	}
	for (int i = 0; i < output_samples; i += jump) {
		for (int j = 0; j < 32; j++) {
			fout_y << _fingers[i][j] << "\t";
		}
		fout_y << endl;
	}
	fout_x.close();
	fout_y.close();
}

void FingerExtractor::GetQueryFinger(FingerItem* new_finger, int& size) {
	size = _frame_num;
	for (int i = 0; i < _frame_num; i++) {
		new_finger[i] = bitset<32>(_fingers[i]);
	}
	return;
}

int FingerExtractor::GetFingerFileId() {
	string originFile = _wavepath.substr(_wavepath.find_last_of("\\") + 1, _wavepath.size());
	return stoi(originFile.substr(0, originFile.find(".")));
}

int FingerExtractor::PrintFingerToFile(const string fingerFile) {
	FILE *fp = fopen(fingerFile.c_str(), "w");
	string sub_finger;
	for (int i = 0; i < _frame_num; i++) {
		sub_finger = "";
		for (int j = 0; j < 32; j++)
			sub_finger.push_back(_fingers[i][j]);
		bitset<32> b(sub_finger);
		fprintf(fp, "%lu\n", b.to_ulong());
	}
	fclose(fp);
	return 0;
}