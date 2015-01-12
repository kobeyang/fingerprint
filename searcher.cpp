#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <cmath>
#include <ctime>
#include <forward_list>
#include <algorithm>
#include <thread>
#include "searcher.h"
#include "util.h"
#include "type.h"

using namespace std;

int hit_number = 0;

bool comp(pair<unsigned int, MusicInfo> a,
	pair<unsigned int, MusicInfo> b) {
	return a.first < b.first;
}

long long Searcher::_BinarySearch(unsigned int key) {
	long long start = 0;
	long long end = _index.size() - 1;
	long long mid;
	while (end >= start) {
		mid = start + (end - start) / 2;
		if (key < _index[mid].first)	{
			end = mid - 1;
		} else if (key > _index[mid].first) {
			start = mid + 1;
		} else {
			return mid;
		}
	}
	return -1;
}

int Searcher::BuildIndex(string dirPath) {
	_finger_database.clear();
	_finger_database.resize(DATABASE_SIZE);
	time_t sort_start, sort_end;
	_allFiles = Util::LoadDir(FINGER_ROOTPATH, "txt");
	for (int i = 0; i < (signed)_allFiles.size(); i++) {
		if (i % 1000 == 0)
			cout << "Index: " << i << endl;
		_BuildOneFileIndex(_allFiles[i]);
	}

	//cout<<"index size: "<<index.size()<<endl;
	sort_start = clock();
	sort(_index.begin(), _index.end(), comp);
	sort_end = clock();
	double sort_time = (double)(sort_end - sort_start) / CLOCKS_PER_SEC;
	cout << "Sort time: " << sort_time << endl;
	return 0;
}

int Searcher::_InsertOneItem(unsigned int key, MusicInfo& m) {
	if (key == 0)
		return 0;
	_index.push_back(make_pair(key, m));
	return 0;
}

int Searcher::_BuildOneFileIndex(const string filepath) {
	vector<unsigned int> audio_file;
	Util::LoadOneFile(filepath, audio_file);
	string originFile = filepath.substr(filepath.find_last_of("\\") + 1, filepath.find_last_of("."));
	int finger_id = stoi(originFile);
#ifdef SUB_SAMPLING
	vector<bitset<32>> temp_v = Util::VectorIntToVectorBitset(audio_file);
	vector<bitset<32>> fingers_block;
	for (int i = 0; i < temp_v.size(); i++)	{
		if (i % M == 0)
			fingers_block.push_back(temp_v[i]);
	}
	_finger_database[finger_id] = fingers_block;
#else
	finger_database[finger_id] = Util::VectorIntToVectorBitset(audio_file);
#endif
	MusicInfo m(finger_id, 0);
	int i = 0;
	//total_line += audio_file.size();

#ifdef SUB_SAMPLING
	for (i = 0; i < (signed)audio_file.size(); i++)	{
		if (i % M == 0)	{
			m.i_frame = i / M;
			_InsertOneItem(audio_file[i], m);
		}
	}
#else
	for (i = 0; i < (signed)audio_file.size(); i++)// 记录是第几个sub_fingerprint
	{
		m.i_frame = i;
		_insert_one_item(audio_file[i], m);
	}
#endif
	return i;
}

int Searcher::_InnerSearch(unsigned long key, FingerItem* finger_block,
	const int block_size, const int i, map<int, int>* result_map) {
	bool is_find = false;
	//* vector index
	time_t search_start, search_end;
	search_start = clock();
	long long result = _BinarySearch(key);
	if (result == -1)
		return -1;
	long long start = result;
	long long end = result;
	do {
		start--;
	} while (start >= 0 && _index[start].first == key);
	start++;
	do {
		end++;
	} while (end < (signed)_index.size() && _index[end].first == key);
	end--;
	search_end = clock();
	duration_search += (double)(search_end - search_start) / CLOCKS_PER_SEC;
	for (long long iter = start; iter <= end; iter++) {
		double diffbits = _CompareBitsets(_index[iter].second.id,
			finger_block, block_size, i, _index[iter].second.i_frame);
		if (diffbits <= BIT_ERROR_RATE) {
			is_find = true;
			return _index[iter].second.id;
			(*result_map)[diffbits] = _index[iter].second.id;
		}
	}
	return -1;
}

int Searcher::Search(FingerItem* finger_block, const int block_size, int& tmp_dif) {
	map<int, int> result_map;
	FingerItem sub_finger_block[M][SUB_BLOCK_SIZE];
	for (int i = 0; i < block_size; i++) {
		sub_finger_block[i % M][i / M] = finger_block[i];
	}
	int result = -1;
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		for (int k = 0; k < M; k++)	{
			unsigned long key = sub_finger_block[k][i].to_ulong();
			if (key == 0)
				continue;
			result = _InnerSearch(key, sub_finger_block[k], SUB_BLOCK_SIZE, i, &result_map);
			if (result > 0)
				return result;
		}
	}
#ifdef ONE_BIT_SEARCH
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		for (int k = 0; k < M; k++)	{
			for (int j = 0; j < 32; j++) {
				FingerItem item = sub_finger_block[k][i];
				item.flip(j);
				unsigned long key = item.to_ulong();
				if (key == 0)
					continue;
				result = _InnerSearch(key, sub_finger_block[k], SUB_BLOCK_SIZE, i, &result_map);
				if (result > 0)
					return result;
			}
		}
	}

#endif

#ifdef TWO_BIT_SEARCH
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		for (int k = 0; k < M; k++) {
			for (int j = 0; j < 31; j++) {
				for (int m = j + 1; m < 32; m++) {
					FingerItem item = sub_finger_block[k][i];
					item.flip(j);
					item.flip(m);
					unsigned long key = item.to_ulong();
					if (key == 0)
						continue;
					result = _InnerSearch(key, sub_finger_block[k], SUB_BLOCK_SIZE, i, &result_map);
					if (result > 0)
						return result;
				}
			}
		}
	}
#endif

#ifdef THREE_BIT_SEARCH
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		for (int k = 0; k < M; k++)	{
			for (int j = 0; j < 30; j++) {
				for (int m = j + 1; m < 31; m++) {
					for (int n = m + 1; n < 32; n++) {
						FingerItem item = sub_finger_block[k][i];
						item.finger.flip(j);
						item.finger.flip(m);
						item.finger.flip(n);
						unsigned long key = item.finger.to_ulong();
						if (key == 0)
							continue;
						result = _inner_search(key, sub_finger_block[k], SUB_BLOCK_SIZE, i, &result_map);
					}
				}
			}
		}
	}
#endif
	hit_number += result_map.size();
	if (result_map.size() > 0) {
		tmp_dif = result_map.begin()->first;
		return result_map.begin()->second;
	}
	else
		return -1;
}

/*
*i_frame_in_block: 在query指纹块中命中的index
*i_frame_in_file: 在file中命中的index
*/
double Searcher::_CompareBitsets(int id, FingerItem* finger_block, const int block_size, \
	const int i_frame_in_block, int i_frame_in_file) {
	if (i_frame_in_file - i_frame_in_block < 0)
		return INT_MAX;//表示错误，返回1.0
	int diff_bits = 0;
	vector<bitset<32>>& full_audio_fingers = _finger_database[id];

	if (i_frame_in_file + block_size - i_frame_in_block > full_audio_fingers.size())
		return INT_MAX;
	i_frame_in_file -= i_frame_in_block;

	//time_compare_start = clock();

	for (int i = 0; i < block_size; i++) {
		bitset<32> subfinger_xor = finger_block[i] ^ full_audio_fingers[i_frame_in_file];
		i_frame_in_file++;
		diff_bits += (int)subfinger_xor.count();
	}

	//time_compare_finish = clock();
	//duration_compare += (double)(time_compare_finish - time_compare_start)/CLOCKS_PER_SEC;
	return (double)diff_bits / (32 * SUB_BLOCK_SIZE);
}

int Searcher::LoadIndex(string filepath) {
	_index.clear();
	unsigned int index_size = 0;
	ifstream fin(filepath, ios::in | ifstream::binary);
	fin.read(reinterpret_cast<char *>(&index_size), sizeof(int));
	if (index_size == 0) {
		fin.close();
		return -1;
	}
	_index.resize(index_size);
	fin.read(reinterpret_cast<char *>(&_index[0]), _index.size() * sizeof(_index[0]));
	fin.close();
	return 0;
}

int Searcher::LoadFingerDatabase(string filepath_prefix) {
	_finger_database.clear();
	vector<thread> threads(OUTPUT_THREAD);
	for (int i = 0; i < OUTPUT_THREAD; i++)	{
		threads[i] = thread(&Searcher::_LoadFingerFromOneFile, this, filepath_prefix, i);
	}

	for (int i = 0; i < OUTPUT_THREAD; i++) {
		threads[i].join();
	}
	return 0;
}

int Searcher::_LoadFingerFromOneFile(string filepath_prefix, unsigned int fileNum) {
	ifstream fin(filepath_prefix + to_string(fileNum), ios::in | ifstream::binary);
	int databaseSize = 0;
	fin.read(reinterpret_cast<char *>(&databaseSize), sizeof(databaseSize));
	_finger_database.resize(databaseSize);
	unsigned int songID = 0;
	unsigned int fingerSize = 0;
	vector<unsigned int> iv;
	while (true) {
		fin.read(reinterpret_cast<char *>(&songID), sizeof(songID));
		if (fin.eof())
			break;
		fin.read(reinterpret_cast<char *>(&fingerSize), sizeof(fingerSize));
		if (fingerSize != 0) {
			iv.resize(fingerSize);
			fin.read(reinterpret_cast<char *>(&iv[0]), fingerSize * sizeof(iv[0]));
			_finger_database[songID] = Util::VectorIntToVectorBitset(iv);
		}
	}
	fin.close();
	return 0;
}

int Searcher::_OutputFingerToOneFile(string filepath_prefix,
	unsigned int databaseSize, unsigned int fileNum) {
	ofstream fout(filepath_prefix + to_string(fileNum), ios::out | ofstream::binary);
	fout.write(reinterpret_cast<char *>(&databaseSize), sizeof(unsigned int));
	unsigned int fingerSize = 0;
	vector<unsigned int> intVector;
	for (unsigned int i = 0; i < databaseSize; i++)	{
		if (i % OUTPUT_THREAD == fileNum) {
			intVector = Util::VectorBitsetToVectorInt(_finger_database[i]);
			fingerSize = (unsigned int)intVector.size();
			fout.write(reinterpret_cast<char *>(&i), sizeof(i));
			fout.write(reinterpret_cast<char *>(&fingerSize), sizeof(fingerSize));
			if (fingerSize != 0)
				fout.write(reinterpret_cast<char *>(&intVector[0]), fingerSize * sizeof(intVector[0]));
		}
	}
	fout.close();
	return 0;
}

int Searcher::OutputFingerToFile(string filepath_prefix) {
	unsigned int databaseSize = (unsigned int)_finger_database.size();
	vector<thread> threads(OUTPUT_THREAD);

	for (int i = 0; i < OUTPUT_THREAD; i++) {
		threads[i] = thread(&Searcher::_OutputFingerToOneFile, this, filepath_prefix, databaseSize, i);
	}

	for (int i = 0; i < OUTPUT_THREAD; i++)	{
		threads[i].join();
	}
	return 0;
}

int Searcher::OutputIndexToFile(string filepath) {
	ofstream fout(filepath, ios::out | ofstream::binary);
	size_t index_size = _index.size();
	fout.write(reinterpret_cast<char *>(&index_size), sizeof(int));
	if (index_size == 0) {
		fout.close();
		return -1;
	}
	fout.write(reinterpret_cast<char *>(&_index[0]), _index.size() * sizeof(_index[0]));
	fout.close();
	return 0;
}

int Searcher::Clear() {
	_index.clear();
	_finger_database.clear();
	return 0;
}

void Searcher::DoStatistics() {
	int number = 1;
	int distinct_key = 1;
	vector<int> distribution(21, 0);
	for (int i = 1; i < _index.size(); i++) {
		if (_index[i].first != _index[i - 1].first) {
			if (number < 20)
				distribution[number]++;
			else
				distribution[20]++;
			distinct_key++;
			number = 1;
		}
		else {
			number++;
		}
	}
	cout << "Total keys: " << _index.size() << endl;
	cout << "Distinct keys: " << distinct_key << endl;
	for (int i = 1; i < distribution.size(); i++)
		cout << "Key numbers for value list length " << i << ": "
		<< (double)distribution[i] / distinct_key * 100 << "%" << endl;
	cout << "Average length: " << (double)_index.size() / distinct_key << endl;

	vector<int> bits(32, 0);
	for (int i = 0; i < _index.size(); i++) {
		for (int j = 0; j < 32; j++) {
			if ((_index[i].first >> j) & 1)
				bits[j]++;
		}
	}
	for (int i = 0; i < 32; i++)
		cout << "1 in bit " << i << ": " << (double)bits[i] / _index.size() * 100 << "%" << endl;
}