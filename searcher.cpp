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

extern map<int, int> tmp_result[3600];

bool comp(pair<int, MusicInfo> a, pair<int, MusicInfo> b) {
	return a.first < b.first;
}

int Searcher::Clear() {
	index.clear();
	finger_database.clear();
	return 0;
}

long long Searcher::_binary_search(unsigned int key) {
	long long start = 0;
	long long end = index.size() - 1;
	long long mid;
	while (end >= start) {
		mid = start + (end - start) / 2;
		if (key < index[mid].first)	{
			end = mid - 1;
		} else if (key > index[mid].first) {
			start = mid + 1;
		} else {
			return mid;
		}
	}
	return -1;
}

int Searcher::build_index(string dirPath) {
	finger_database.clear();
	finger_database.resize(DATABASE_SIZE);
	time_t sort_start, sort_end;
	allFiles = Util::load_dir(FINGER_ROOTPATH, "txt");
	for (int i = 0; i < (signed)allFiles.size(); i++) {
		if (i % 1000 == 0)
			cout << "Index: " << i << endl;
		_build_one_file_index(allFiles[i]);
	}

	//cout<<"index size: "<<index.size()<<endl;
	sort_start = clock();
	sort(index.begin(), index.end(), comp);
	sort_end = clock();
	double sort_time = (double)(sort_end - sort_start) / CLOCKS_PER_SEC;
	cout << "Sort time: " << sort_time << endl;
	//_do_statistics();
	return 0;
}

int Searcher::_insert_one_item(unsigned int key, MusicInfo& m) {
	if (key == 0)
		return 0;
	index.push_back(make_pair(key, m));
	return 0;
}

int Searcher::_build_one_file_index(const string filepath) {
	vector<unsigned int> audio_file;
	Util::load_one_file(filepath, audio_file);
	string originFile = filepath.substr(filepath.find_last_of("\\") + 1, filepath.find_last_of("."));
	int finger_id = stoi(originFile);
#ifdef SUB_SAMPLING
	vector<bitset<32>> temp_v = Util::VectorIntToVectorBitset(audio_file);
	vector<bitset<32>> fingers_block;
	for (int i = 0; i < temp_v.size(); i++)	{
		if (i % M == 0)
			fingers_block.push_back(temp_v[i]);
	}
	finger_database[finger_id] = fingers_block;
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
			_insert_one_item(audio_file[i], m);
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

/*
void Searcher::_test_index()
{
for(int i = 1; i < 3; i++)
{
forward_list<MusicInfo> list = array_index[i];
for(forward_list<MusicInfo>::iterator iter = list.begin(); iter != list.end(); iter++)
{
cout<<iter->id<<"\t"<<iter->i_frame<<endl;
}
cout<<endl;
}
}
*/

int Searcher::_inner_search(unsigned int queryID, unsigned long key,
	FingerItem* finger_block, const int block_size, const int i, int& tmp_dif) {
	bool is_find = false;
	//* vector index
	time_t search_start, search_end;
	search_start = clock();
	long long result = _binary_search(key);
	if (result == -1)
		return -1;
	long long start = result;
	long long end = result;
	do {
		start--;
	} while (start >= 0 && index[start].first == key);
	start++;
	do {
		end++;
	} while (end < (signed)index.size() && index[end].first == key);
	end--;
	search_end = clock();
	duration_search += (double)(search_end - search_start) / CLOCKS_PER_SEC;
	for (long long iter = start; iter <= end; iter++) {
		int diffbits = compare_bitsets(index[iter].second.id, finger_block, block_size, i, index[iter].second.i_frame);
		if (diffbits <= THREHOLD_BITS) {
			diff += diffbits;
			match++;
			//cout<<"result frame: "<< index[iter].second.i_frame<<"---";
			is_find = true;
			tmp_dif = diffbits;
			tmp_result[queryID][tmp_dif] = index[iter].second.id;
			//return index[iter].second.id;
		}
	}
	/// vector index end
	return -1;
}

/*
int Searcher::search(FingerItem* finger_block, const int block_size, int& temp_dif)
{
int result = -1;
//exact match
for(int i = 0; i < block_size; i++)//从查询指纹块的第一条指纹开始
{
unsigned long key = finger_block[i].finger.to_ulong();
if(key == 0)
continue;
result = _inner_search(key, finger_block, block_size, i, temp_dif);
if(result != -1)
{
hit_index[i]++; //表示从第i条指纹中找到
return result;
}
}

//near match
#ifdef OPEN_NEAR_SEARCH
for(int i = 0; i < block_size; i++)
{
FingerItem item = finger_block[i];
for(int try_count = 0; try_count < (int)pow(2, TOGGLENUM); try_count++)
{
bitset<TOGGLENUM> tempbit(try_count);
for(int bit_index = 0; bit_index < TOGGLENUM; bit_index++)
{
if(tempbit.test(bit_index))
item.finger.set(item.toggle_bits[bit_index]);
else
item.finger.reset(item.toggle_bits[bit_index]);
}
unsigned long key = item.finger.to_ulong();
if(key == 0)
continue;
result = _inner_search(key, finger_block, block_size, i, temp_dif);
if(result != -1)
return result;
}
}
#endif

#ifdef ONE_BIT_SEARCH
for(int i = 0; i < block_size; i++)
{
for(int j = 0; j < 32; j++)
{
FingerItem item = finger_block[i];
item.finger.flip(j);
unsigned long key = item.finger.to_ulong();
if(key == 0)
continue;
result = _inner_search(key, finger_block, block_size, i, temp_dif);
if(result != -1)
return result;
}
}
#endif

#ifdef TWO_BIT_SEARCH
for(int i = 0; i < block_size; i++)
{
for(int j = 0; j < 31; j++)
{
for(int k = j + 1; k < 32; k++)
{
FingerItem item = finger_block[i];
item.finger.flip(j);
item.finger.flip(k);
unsigned long key = item.finger.to_ulong();
if(key == 0)
continue;
result = _inner_search(key, finger_block, block_size, i, temp_dif);
if(result != -1)
return result;
}
}
}
#endif

#ifdef THREE_BIT_SEARCH
for(int i = 0; i < block_size; i++)
{
for(int j = 0; j < 30; j++)
{
for(int k = j + 1; k < 31; k++)
{
for(int m = k + 1; m < 32; m++)
{
FingerItem item = finger_block[i];
item.finger.flip(j);
item.finger.flip(k);
item.finger.flip(m);
unsigned long key = item.finger.to_ulong();
if(key == 0)
continue;
result = _inner_search(key, finger_block, block_size, i, temp_dif);
if(result != -1)
return result;
}
}
}
}
#endif
return -1;
}
*/

int Searcher::SubSamplingSearch(unsigned int queryID, FingerItem* finger_block, const int block_size, int& tmp_dif)
{
	FingerItem sub_finger_block[M][SUB_BLOCK_SIZE];
	for (int i = 0; i < block_size; i++) {
		sub_finger_block[i % M][i / M] = finger_block[i];
	}
	int result = -1;
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		for (int k = 0; k < M; k++)	{
			unsigned long key = sub_finger_block[k][i].finger.to_ulong();
			if (key == 0)
				continue;
			result = _inner_search(queryID, key, sub_finger_block[k], SUB_BLOCK_SIZE, i, tmp_dif);
			if (result != -1) {
				tmp_result[queryID][tmp_dif] = result;
				//return result;
			}
		}
	}
#ifdef ONE_BIT_SEARCH
	for (int i = 0; i < SUB_BLOCK_SIZE; i++) {
		for (int k = 0; k < M; k++)	{
			for (int j = 0; j < 32; j++) {
				FingerItem item = sub_finger_block[k][i];
				item.finger.flip(j);
				unsigned long key = item.finger.to_ulong();
				if (key == 0)
					continue;
				result = _inner_search(queryID, key, sub_finger_block[k], SUB_BLOCK_SIZE, i, tmp_dif);
				if (result != -1) {
					tmp_result[queryID][tmp_dif] = result;
					//return result;
				}
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
					item.finger.flip(j);
					item.finger.flip(m);
					unsigned long key = item.finger.to_ulong();
					if (key == 0)
						continue;
					result = _inner_search(queryID, key, sub_finger_block[k], SUB_BLOCK_SIZE, i, tmp_dif);
					if (result != -1) {
						tmp_result[queryID][tmp_dif] = result;
						//return result;
					}
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
						result = _inner_search(key, sub_finger_block[k], SUB_BLOCK_SIZE, i, tmp_dif);
						if (result != -1)
							return result;
					}
				}
			}
		}
	}
#endif

	cout << "SIZE: " << tmp_result[queryID].size() << endl;
	if (tmp_result[queryID].size() != 0) {
		tmp_dif = tmp_result[queryID].begin()->first;
		return tmp_result[queryID].begin()->second;
	}
	else
		return -1;
}

/*
*i_frame_in_block: 在query指纹块中命中的index
*i_frame_in_file: 在file中命中的index
*/
int Searcher::compare_bitsets(int id, FingerItem* finger_block, const int block_size, \
	const int i_frame_in_block, int i_frame_in_file) {
	if (i_frame_in_file - i_frame_in_block < 0)
		return INT_MAX;//表示错误，返回1.0
	int diff_bits = 0;
	vector<bitset<32>>& full_audio_fingers = finger_database[id];

	if (i_frame_in_file + block_size - i_frame_in_block > full_audio_fingers.size())
		return INT_MAX;
	i_frame_in_file -= i_frame_in_block;

	//time_compare_start = clock();

	for (int i = 0; i < block_size; i++) {
		bitset<32> subfinger_xor = finger_block[i].finger ^ full_audio_fingers[i_frame_in_file];
		i_frame_in_file++;
		diff_bits += (int)subfinger_xor.count();
	}

	//time_compare_finish = clock();
	//duration_compare += (double)(time_compare_finish - time_compare_start)/CLOCKS_PER_SEC;
	return diff_bits;
}


double Searcher::get_mean_diff() {
	return diff / match;
}

int Searcher::LoadIndex(string filepath) {
	index.clear();
	unsigned int index_size = 0;
	ifstream fin(filepath, ios::in | ifstream::binary);
	fin.read(reinterpret_cast<char *>(&index_size), sizeof(int));
	if (index_size == 0) {
		fin.close();
		return -1;
	}
	index.resize(index_size);
	fin.read(reinterpret_cast<char *>(&index[0]), index.size() * sizeof(index[0]));
	fin.close();
	return 0;
}

int Searcher::_loadFingerFromOneFile(string filepath_prefix, unsigned int fileNum) {
	ifstream fin(filepath_prefix + to_string(fileNum), ios::in | ifstream::binary);
	int databaseSize = 0;
	fin.read(reinterpret_cast<char *>(&databaseSize), sizeof(databaseSize));
	finger_database.resize(databaseSize);
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
			finger_database[songID] = Util::VectorIntToVectorBitset(iv);
		}
	}
	fin.close();
	return 0;
}

int Searcher::LoadFingerDatabase(string filepath_prefix) {
	finger_database.clear();
	vector<thread> threads(OUTPUT_THREAD);
	for (int i = 0; i < OUTPUT_THREAD; i++)	{
		threads[i] = thread(&Searcher::_loadFingerFromOneFile, this, filepath_prefix, i);
	}

	for (int i = 0; i < OUTPUT_THREAD; i++) {
		threads[i].join();
	}
	return 0;
}

int Searcher::_outputFingerToOneFile(string filepath_prefix,
	unsigned int databaseSize, unsigned int fileNum) {
	ofstream fout(filepath_prefix + to_string(fileNum), ios::out | ofstream::binary);
	fout.write(reinterpret_cast<char *>(&databaseSize), sizeof(unsigned int));
	unsigned int fingerSize = 0;
	vector<unsigned int> intVector;
	for (unsigned int i = 0; i < databaseSize; i++)	{
		if (i % OUTPUT_THREAD == fileNum) {
			intVector = Util::VectorBitsetToVectorInt(finger_database[i]);
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
	unsigned int databaseSize = (unsigned int)finger_database.size();
	vector<thread> threads(OUTPUT_THREAD);

	for (int i = 0; i < OUTPUT_THREAD; i++) {
		threads[i] = thread(&Searcher::_outputFingerToOneFile, this, filepath_prefix, databaseSize, i);
	}

	for (int i = 0; i < OUTPUT_THREAD; i++)	{
		threads[i].join();
	}
	return 0;
}


int Searcher::OutputIndexToFile(string filepath) {
	ofstream fout(filepath, ios::out | ofstream::binary);
	size_t index_size = index.size();
	fout.write(reinterpret_cast<char *>(&index_size), sizeof(int));
	if (index_size == 0) {
		fout.close();
		return -1;
	}
	fout.write(reinterpret_cast<char *>(&index[0]), index.size() * sizeof(index[0]));
	fout.close();
	return 0;
}