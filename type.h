#pragma once
#include <bitset>
#include <vector>
#include <string>

//#define OPEN_NEAR_SEARCH
#define ONE_BIT_SEARCH
#define TWO_BIT_SEARCH
//#define THREE_BIT_SEARCH

class FingerItem {
public:
	std::bitset<32> finger;
#ifdef OPEN_NEAR_SEARCH
	std::vector<short> toggle_bits;
#endif
};

class MusicInfo {
public:
	int id;
	int i_frame;
	MusicInfo(int ID, int FID) :id(ID), i_frame(FID){};
	MusicInfo(){};
};

typedef std::vector<std::pair<unsigned int, MusicInfo>> IndexType;

const int NumSamplesPerFrameM = 1850;

const int DATABASE_SIZE = 174000;
const double THRESHOLD = 0.45;//0.4289;
//const int TOGGLENUM = 3;
const int FFT_THREAD = 1;
const int OUTPUT_THREAD = 10;
const int THREAD_NUM = 10;
const int SUB_FINGER_NUM = 380000; // there are 186056 subfingerprints in 90408 with 23.2
// 115 for 3 seconds, 204 for 5 seconds with hop 23.2 ms,
// 409 for 5 seconds, 500 for 6 seconds with hop 11.6 ms.
const int QUERY_FINGER_NUM = 204;

#define SUB_SAMPLING
const int M = 1;
const int SUB_BLOCK_SIZE = QUERY_FINGER_NUM / M;

#ifdef SUB_SAMPLING
const int THREHOLD_BITS = (int)(THRESHOLD * 32 * SUB_BLOCK_SIZE);
#else
const int THREHOLD_BITS = (int)(THRESHOLD * 32 * QUERY_FINGER_NUM);
#endif

#define SEARCH
//#define MULTITHREAD_FFT

//const std::string WAVE_ROOTPATH = "Z:\\200000_s48_24000hz_wav\\";
const std::string WAVE_ROOTPATH = "Z:\\200000_s48_24000hz_wav\\";
const std::string FINGER_ROOTPATH = "E:\\yangguang\\FingerprintingExtraction\\200000_s48_24000hz_wav\\fingers_232";
const std::string QUERY_WAVE_PATH = "E:\\yangguang\\FingerprintingExtraction\\200000_s48_24000hz_wav\\new_broadcast_query\\1";//1_5seconds_wave_96"; //里面存的是wave文件
const std::string INDEX_FILE_PATH = "E:\\yangguang\\FingerprintingExtraction\\200000_s48_24000hz_wav\\index\\index_170_232";
const std::string WHOLE_FINGER_PATH = "E:\\yangguang\\FingerprintingExtraction\\200000_s48_24000hz_wav\\index\\finger_170_232";
//const std::string QUERY_FINGER_PATH = "E:\\yangguang\\FingerprintingExtraction\\3minutesfinger";