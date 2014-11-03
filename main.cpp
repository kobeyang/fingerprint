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

Searcher searcher;
int yes = 0;//记录正确搜索的音频
int not_found = 0;
int hit_index[500];
pair<int, int> final_result[3600];
map<int, int> tmp_result[3600];

double duration_analyze, duration_search;
double duration_not_find;
double duration_compare;
double duration_FFT;

double total_dif;

ofstream fout_same;
ofstream fout_nf;
ofstream fout_find;
set<int> s_same, s_nf;

void SearchOneFile(vector<string>& allQueryFiles) {
	FingerExtractor extractor;
	clock_t time_analyze_start, time_analyze_finish;
	clock_t time_search_start, time_search_finish;
	for (int i = 0; i < (signed)allQueryFiles.size(); i++) {
		time_analyze_start = clock();
		//cout<<allQueryFiles[i]<<endl;
		extractor.CalcFingerprint(QUERY_WAVE_PATH + "\\" + allQueryFiles[i]);

		//string filename = allQueryFiles[i].substr(0, allQueryFiles[i].find("."));
		//cout<<"file: "<<filename<<endl;
		//getchar();
		//extractor.PrintFingerToFile(QUERY_FINGER_PATH + "\\" + filename + ".txt");
		FingerItem finger_block[QUERY_FINGER_NUM];
		int size = 0;
		extractor.getQueryFinger(finger_block, size);

		int queryId = extractor.getFingerFileId();
		/*
		if(s_nf.find(queryId) != s_nf.end())
		continue;
		*/
		time_analyze_finish = clock();
		time_search_start = clock();
		int tmp_dif = 0;
#ifdef SUB_SAMPLING
		int result = searcher.SubSamplingSearch(queryId, finger_block, QUERY_FINGER_NUM, tmp_dif);
#else
		int result = searcher.search(finger_block, QUERY_FINGER_NUM, temp_dif);
#endif
		//int result = -1;
		if (result == -1) {
			//cout<<"file: "<<queryId<<" Not found"<<endl;
			not_found++;
			time_search_finish = clock();
			duration_not_find += (double)(time_search_finish - time_search_start) / CLOCKS_PER_SEC;
			fout_nf << (to_string(queryId) + "\n");
		}
		//这里比较的是得到的finger.txt的ID和原始wavefile的ID，由于目前这两个ID一样，所以可以这样比
		/*
		else if(result == queryId)
		{
		time_search_finish = clock();
		duration_compare += (double)(time_search_finish - time_search_start)/CLOCKS_PER_SEC;
		//cout<<"Match!  "<<result<<endl;
		//cout<<queryId<<"\t"<<temp_dif<<endl;
		yes++;
		}
		*/
		else
		{
			yes++;
			total_dif += tmp_dif;
			//cout<<queryId<<"\t"<<temp_dif<<endl;
			time_search_finish = clock();
			duration_compare += (double)(time_search_finish - time_search_start) / CLOCKS_PER_SEC;
			;//cout<<"Not match!"<<endl;
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

void ExtractFingerprint(vector<string>& allFiles, int dirNum) {
	// 分析wave歌曲，提取指纹
	FingerExtractor extractor;

	//clock_t extractStart,extractEnd;
	//extractStart = clock();

	for (int i = 0; i < (signed)allFiles.size(); i++) {
		size_t pos = allFiles[i].find_last_of("\\");
		string temp = allFiles[i].substr(pos + 1, allFiles[i].size() - pos);
		string filename = temp.substr(0, temp.find("."));

		//string filename = allFiles[i].substr(0, allFiles[i].find("."));

		fstream _file;
		_file.open(FINGER_ROOTPATH + "\\" + filename + ".txt", ios::in);
		if (_file) {
			cout << "文件已存在 " << filename << endl;
			_file.close();
			continue;
		}

		cout << filename << endl;
		if (dirNum == -1) {
			//extractor.AnalyzeFingerprint(WAVE_ROOTPATH + "\\" + allFiles[i]);
			//extractor.CalcFingerprint(WAVE_ROOTPATH + "\\" + allFiles[i]);
			extractor.CalcFingerprint(allFiles[i]);
			extractor.PrintFingerToFile(FINGER_ROOTPATH + "\\" + filename + ".txt");
		} else {
			extractor.CalcFingerprint(WAVE_ROOTPATH + to_string(dirNum) + "\\" + allFiles[i]);
			extractor.PrintFingerToFile(FINGER_ROOTPATH + "\\" + filename + ".txt");
		}
		//cout<<"File: "<<allFiles[i]<<endl;
	}
	//extractEnd = clock();
	//double extract_duration = (double)(extractEnd - extractStart)/CLOCKS_PER_SEC;
	//cout<<"Time: "<<extract_duration<<endl;
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

void PostProcess() {
	int last_song = final_result[0].first;

	//cout<<"---Origion result---"<<endl;
	for (int i = 1; i < 3600; i++) {
		if (final_result[i].first != 0)	{
			last_song = final_result[i].first;
		} else {
			if (final_result[i - 1].first == 0)
				continue;
			int j = i;
			for (; (j < i + 5) && j < 3600; j++) {
				if (final_result[j].first == 0)
					continue;
				if (final_result[j].first == last_song) {
					for (int k = i; k < j; k++)	{
						final_result[k].first = last_song;
					}
					break;
				} else { // a new song
					last_song = final_result[j].first;
					break;
				}
			}
			i = j;
		}
	}

	//cout<<"---After first process---"<<endl;
	last_song = -1;
	int start = 0;
	int end = 0;
	//output statistics result
	for (int i = 0; i < 3600; i++) {
		if (final_result[i].first != 0) {
			if (final_result[i].first == last_song) {
				end = i;
			} else {
				if (last_song != -1 && start < end) // start < end , exclude songs appear 1 second
					fout_find << last_song % 173000 << "\t" << FromIntToTime(start) << "\t" << FromIntToTime(end) << endl;
				start = i;
				last_song = final_result[i].first;
			}
		} else {
			if (last_song == -1) {
				continue;
			} else {
				if (start < end) // start < end , exclude songs appear 1 second
					fout_find << last_song % 173000 << "\t" << FromIntToTime(start) << "\t" << FromIntToTime(end) << endl;
				last_song = -1;
			}
		}
	}
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

void PrintFinalResult() {
	for (int i = 0; i < 3600; i++) {
		if (final_result[i].first != 0)
			fout_find << FromIntToTime(i) << "\t" << to_string(final_result[i].first) << "\t" << to_string(final_result[i].second) << endl;
	}
	return;
}

int main(int argc, char* argv[]) {
#ifndef SEARCH
	/*
	for(int dirNum = 1; dirNum < 10; dirNum++)
	{
	vector<vector<string>> allQueryFiles(THREAD_NUM);
	for(int i = 0; i < hehaiqianFiles.size(); i++)
	{
	allQueryFiles[i%THREAD_NUM].push_back(hehaiqianFiles[i]);
	}
	//Util::load_dir_specific(allQueryFiles, WAVE_ROOTPATH + to_string(dirNum), "wav");
	//Util::load_dir_specific(allQueryFiles, WAVE_ROOTPATH, "wav");
	vector<thread> threads;

	for(int i = 0; i < THREAD_NUM; i++)
	{
	threads.push_back(thread(ExtractFingerprint, allQueryFiles[i], -1));
	}
	for(int i = 0; i < THREAD_NUM; i++)
	{
	threads[i].join();
	}
	}
	cout<<"Extract Done!"<<endl;
	getchar();
	*/
#else
	/*
	searcher.build_index(FINGER_ROOTPATH);
	searcher.OutputIndexToFile(INDEX_FILE_PATH);
	searcher.OutputFingerToFile(WHOLE_FINGER_PATH);
	cout<<"build index done"<<endl;
	getchar();
	*/
	double duration = 0;
	clock_t start, mid, finish;
	start = clock();
	searcher.LoadIndex(INDEX_FILE_PATH);
	mid = clock();
	cout << "Load Index: " << (double)(mid - start) / CLOCKS_PER_SEC << endl;
	searcher.LoadFingerDatabase(WHOLE_FINGER_PATH);
	finish = clock();
	duration = (double)(finish - mid) / CLOCKS_PER_SEC;
	cout << "Load Database: " << duration << endl;
	cout << "Build index done!" << endl;

	fout_nf.open("E:\\yangguang\\FingerprintingExtraction\\200000_s48_24000hz_wav\\nf.txt", ios::out);
	fout_same.open("E:\\yangguang\\FingerprintingExtraction\\200000_s48_24000hz_wav\\same.txt", ios::out);
	fout_find.open("E:\\yangguang\\FingerprintingExtraction\\200000_s48_24000hz_wav\\result.txt", ios::out);

	not_found = 0;

	vector<vector<string>> allQueryFiles(THREAD_NUM);
	Util::load_dir_specific(allQueryFiles, QUERY_WAVE_PATH, "wav");
	vector<thread> threads;

	start = clock();

	for (int i = 0; i < THREAD_NUM; i++) {
		threads.push_back(thread(SearchOneFile, allQueryFiles[i]));
	}
	for (int i = 0; i < THREAD_NUM; i++) {
		threads[i].join();
	}
	searcher.Clear();
	//PostProcess();
	PrintFinalResult();
	finish = clock();
	duration = (double)(finish - start) / CLOCKS_PER_SEC;
	cout << "Time: " << duration << endl;
	//cout<<"Analyze time: "<<duration_analyze<<endl;
	//cout<<"FFT time: "<<duration_FFT<<endl;
	//cout<<"Search time: "<<duration_search<<endl;
	//cout<<"Compare time: "<<duration_compare<<endl;
	//cout<<"Not found time: "<<duration_not_find<<endl;
	fout_same << "yes: " << yes << endl;
	fout_same << "total dif: " << total_dif << endl;
	fout_same << "threshold: " << total_dif / yes << endl;
	//cout<<"not found: "<<not_found<<endl;
	//cout<<0<<"Recall rate:"<<(double)(10000-not_found)/10000<<endl;
	fout_nf.close();
	fout_same.close();
	fout_find.close();
#endif	
	//getchar();
}