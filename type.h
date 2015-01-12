#pragma once
#include <bitset>
#include <vector>
#include <string>

#define SEARCH

#define ONE_BIT_SEARCH
#define TWO_BIT_SEARCH
//#define THREE_BIT_SEARCH

class MusicInfo {
public:
	int id;
	int i_frame;
	MusicInfo(int ID, int FID) :id(ID), i_frame(FID){};
	MusicInfo(){};
};

typedef std::vector<std::pair<unsigned int, MusicInfo>> IndexType;
typedef std::bitset<32> FingerItem;

const int NumSamplesPerFrameM = 1850;
const int DATABASE_SIZE = 174000;
const double BIT_ERROR_RATE = 0.35;
const int FFT_THREAD = 1;
const int OUTPUT_THREAD = 10;
const int THREAD_NUM = 1;
const int SUB_FINGER_NUM = 380000; // there are 186056 subfingerprints in 90408 with 23.2
const double TIME_INTERVAL = 0.0116;
//11.6 ms: 409 for 5 seconds, 500 for 6 seconds, 845 for 10 seconds.
//23.2 ms: 115 for 3 seconds, 204 for 5 seconds.
const int QUERY_FINGER_NUM = 845;

#define SUB_SAMPLING
const int M = 1;
const int SUB_BLOCK_SIZE = QUERY_FINGER_NUM / M;

//const std::string WAVE_ROOTPATH = "Z:\\200000_s48_24000hz_wav\\";
const std::string WAVE_ROOTPATH = "E:\\yangguang\\fingerprint\\a";
const std::string FINGER_ROOTPATH = "E:\\yangguang\\fingerprint\\fingers";
const std::string QUERY_WAVE_PATH = "E:\\yangguang\\cvaf\\data\\query\\record_wave";
const std::string INDEX_FILE_PATH = "E:\\yangguang\\fingerprint\\index\\20k_index";
const std::string WHOLE_FINGER_PATH = "E:\\yangguang\\fingerprint\\index\\20k_finger";
//const std::string QUERY_FINGER_PATH = "E:\\yangguang\\FingerprintingExtraction\\3minutesfinger";