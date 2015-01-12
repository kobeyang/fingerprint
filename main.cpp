#include <iostream>
#include <fstream>
#include <thread>
#include <vector>
#include <bitset>
#include <string>
#include <map>
#include <ctime>
#include "fingerprint-extractor.h"
#include "searcher.h"
#include "util.h"
#include "type.h"

using namespace std;

extern int hit_number;
Searcher searcher;
int yes = 0;//记录正确搜索的音频
int not_found = 0;
int hit_index[500];
pair<int, int> final_result[3600];

double duration_analyze, duration_search;
double duration_not_find;
double duration_compare;
double duration_FFT;

double total_dif;
set<int> s_same, s_nf;
fstream fout;

void SearchOneFile(vector<string>& allQueryFiles) {
	FingerExtractor extractor;
	clock_t time_analyze_start, time_analyze_finish;
	clock_t time_search_start, time_search_finish;
	for (int i = 0; i < (signed)allQueryFiles.size(); i++) {
		time_analyze_start = clock();
		//cout<<allQueryFiles[i]<<endl;
		extractor.CalcFingerprint(allQueryFiles[i]);
		FingerItem finger_block[QUERY_FINGER_NUM];
		int size = 0;
		extractor.GetQueryFinger(finger_block, size);
		int queryId = extractor.GetFingerFileId();
		time_analyze_finish = clock();
		time_search_start = clock();
		int tmp_dif = 0;
#ifdef SUB_SAMPLING
		int result = searcher.Search(finger_block, QUERY_FINGER_NUM, tmp_dif);
#else
		int result = searcher.Search(finger_block, QUERY_FINGER_NUM, temp_dif);
#endif
		//int result = -1;
		if (result == -1) {
			cout<<"file: "<<queryId<<" Not found"<<endl;
			fout << queryId << endl;
			not_found++;
			time_search_finish = clock();
			duration_not_find += (double)(time_search_finish - time_search_start) / CLOCKS_PER_SEC;
			//fout_nf << (to_string(queryId) + "\n");
		}
		//这里比较的是得到的finger.txt的ID和原始wavefile的ID，由于目前这两个ID一样，所以可以这样比
		else if(result == queryId) {
			time_search_finish = clock();
			duration_compare += (double)(time_search_finish - time_search_start)/CLOCKS_PER_SEC;
			//cout<<"Match!  "<<result<<endl;
			//cout<<queryId<<"\t"<<temp_dif<<endl;
			yes++;
		} else {
			total_dif += tmp_dif;
			cout<<"Not match!"<<endl;
			cout<<queryId<<"\t"<<result<<endl;
			time_search_finish = clock();
			duration_compare += (double)(time_search_finish - time_search_start) / CLOCKS_PER_SEC;
			
			final_result[queryId].first = result;
			final_result[queryId].second = tmp_dif;
			//fout_same<<(to_string(queryId) + "\t" + blockId + "\t" + to_string(result) + "\t0" + "\n");
			//fout_find<<(to_string(queryId) + "\n" + to_string(result) + "\n");
			//fout_nf<<(to_string(queryId) + "\n" + to_string(result) + "\n");
		}
		//duration_accessmap += (double)(time_search_finish - time_search_start)/CLOCKS_PER_SEC;
		duration_analyze += (double)(time_analyze_finish - time_analyze_start) / CLOCKS_PER_SEC;
	}
}

void ExtractFingerprint(vector<string>& allFiles) {
	// 分析wave歌曲，提取指纹
	FingerExtractor extractor;
	string x_path = "E:\\yangguang\\fingerprint\\a\\train_x.txt";
	string y_path = "E:\\yangguang\\fingerprint\\a\\train_y.txt";
	for (int i = 0; i < (signed)allFiles.size(); i++) {
		size_t pos = allFiles[i].find_last_of("\\");
		string temp = allFiles[i].substr(pos + 1, allFiles[i].size() - pos);
		string filename = temp.substr(0, temp.find("."));
		/*
		fstream _file;
		_file.open(FINGER_ROOTPATH + "\\" + filename + ".txt", ios::in);
		if (_file) {
			cout << "文件已存在 " << filename << endl;
			_file.close();
			continue;
		}
		*/
		cout << "File: " << allFiles[i] << endl;
		extractor.CalcFingerprint(allFiles[i]);
		extractor.OutputTrainingSamples(x_path, y_path);
		//extractor.PrintFingerToFile(FINGER_ROOTPATH + "\\" + filename + ".txt");
	}
}

string FromIntToTime(int time) {
	int q = time / 60;
	int r = time - (time / 60) * 60;
	string min = to_string(q);
	string sec = to_string(r);
	if (min.size() == 1)
		min = "0" + min;
	if (sec.size() == 1)
		sec = "0" + sec;
	return min + ":" + sec;
}

void LoadSet(string filepath, set<int>& s) {
	ifstream fin(filepath, ios::in);
	int songID;
	while (true) {
		fin >> songID;
		if (fin.eof())
			break;
		s.insert(songID);
	}
	fin.close();
	return;
}

int main(int argc, char* argv[]) {
#ifndef SEARCH
	vector<string> all_files = Util::load_dir(WAVE_ROOTPATH, "wav");
	ExtractFingerprint(all_files);
	cout<<"Extract Done!"<<endl;
	getchar();
#else
	searcher.BuildIndex(FINGER_ROOTPATH);
	searcher.DoStatistics();
	getchar();
	//searcher.OutputIndexToFile(INDEX_FILE_PATH);
	//searcher.OutputFingerToFile(WHOLE_FINGER_PATH);
	//cout << "build index done" << endl;
	double duration = 0;
	clock_t start, mid, finish;
	start = clock();
	searcher.LoadIndex(INDEX_FILE_PATH);
	mid = clock();
	cout << "Load Index: " << (double)(mid - start) / CLOCKS_PER_SEC << endl;
	searcher.LoadFingerDatabase(WHOLE_FINGER_PATH);
	finish = clock();
	cout << "Load Database: " << (double)(finish - mid) / CLOCKS_PER_SEC << endl;
	cout << "Build index done!" << endl;

	//fout.open("E:\\yangguang\\FingerprintingExtraction\\200000_s48_24000hz_wav\\not_found.txt", fstream::out);
	vector<vector<string>> allQueryFiles(THREAD_NUM);
	Util::LoadDirSpecific(allQueryFiles, QUERY_WAVE_PATH, "wav");
	vector<thread> threads;

	start = clock();
	for (int i = 0; i < THREAD_NUM; i++) {
		threads.push_back(thread(SearchOneFile, allQueryFiles[i]));
	}
	for (int i = 0; i < THREAD_NUM; i++) {
		threads[i].join();
	}
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "Yes: " << yes << endl;
	cout << "Not found: " << not_found << endl;
	cout << "Time: " << duration << endl;
	cout << "Hit size: " << hit_number << endl;
	//fout.close();
	getchar();
	//cout<<"Analyze time: "<<duration_analyze<<endl;
	//cout<<"FFT time: "<<duration_FFT<<endl;
	//cout<<"Search time: "<<duration_search<<endl;
	//cout<<"Compare time: "<<duration_compare<<endl;
	//cout<<"Not found time: "<<duration_not_find<<endl;
	//cout<<"not found: "<<not_found<<endl;
	//cout<<0<<"Recall rate:"<<(double)(10000-not_found)/10000<<endl;
#endif	
	//getchar();
}