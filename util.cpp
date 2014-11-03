#include <iostream>
#include <fstream>
#include <vector>
#include <bitset>
#include <string>
#include <thread>
#include <algorithm>
#include <io.h>
#include <fcntl.h>
#include "util.h"
#include "type.h"

using namespace std;

static const int SIZE = 400000 * 33 + 1;
char buffer[SIZE];

std::vector<std::string> Util::allFiles;
std::vector<std::vector<std::bitset<32>>> Util::finger_database;

bool comp(pair<int, MusicInfo> a, pair<int, MusicInfo> b);

int Util::load_one_file(string filepath, vector<unsigned int>& audio_fingers) {
	FILE* fp = fopen(filepath.c_str(), "r");
	if (fp == NULL)	{
		std::cout << "no such file: " << filepath << std::endl;
		return -1;
	}
	audio_fingers.clear();
	char line[20];
	unsigned int key = 0;
	while (true) {
		fgets(line, 20, fp);
		// need to implement
		if (feof(fp))
			break;
		string s(line);
		s = s.substr(0, (signed)s.size() - 1);
		audio_fingers.push_back(stoul(s));
	}
	fclose(fp);
	return 0;
}

/*
int Util::load_one_file(string filepath, vector<unsigned int>& audio_fingers)
{
int fd = _open(filepath.c_str(), O_RDONLY);
audio_fingers.clear();
int size = 0;
while((size = _read(fd, buffer, SIZE - 1)) > 0)
{
buffer[size]='\0';
string alldata(buffer);
for(int i = 0; i < size; i+=33)
{
string s(alldata, i, 32);
std::bitset<32> b(s);
audio_fingers.push_back(b);
}
}
_close(fd);
return 0;
}
*/

std::vector<std::string> Util::load_dir(std::string dirpath, std::string type) {
	allFiles.clear();
	std::string temppath = dirpath + "\\*." + type;
	struct _finddata_t fileinfo;
	intptr_t handle = _findfirst(temppath.c_str(), &fileinfo);
	if (-1 == handle) {
		std::cout << "can not find dir or file!" << std::endl;
	}
	allFiles.push_back(dirpath + "\\" + string(fileinfo.name));
	while (!_findnext(handle, &fileinfo)) {
		allFiles.push_back(dirpath + "\\" + string(fileinfo.name));
	}
	_findclose(handle);
	return allFiles;
}

void Util::load_dir_specific(std::vector<std::vector<std::string>>& allQueryFiles,
	std::string dirpath, std::string type) {
	std::string temppath = dirpath + "\\*." + type;
	struct _finddata_t fileinfo;
	intptr_t handle = _findfirst(temppath.c_str(), &fileinfo);
	if (-1 == handle) {
		std::cout << "can not find dir or file!" << std::endl;
	}
	//std::string path(dirpath + "\\" + fileinfo.name);
	string filename = string(fileinfo.name);
	filename = filename.substr(0, filename.find("."));
	int i_filename = stoi(filename);
	int m = i_filename % THREAD_NUM;
	allQueryFiles[m].push_back(string(fileinfo.name));
	while (!_findnext(handle, &fileinfo)) {
		string filename = string(fileinfo.name);
		filename = filename.substr(0, filename.find("."));
		i_filename = stoi(filename);
		m = i_filename % THREAD_NUM;
		//path = std::string(dirpath + "\\" + fileinfo.name);
		allQueryFiles[m].push_back(string(fileinfo.name));
	}
	_findclose(handle);
	return;
}

int Util::t_DeleteSomeIndex(IndexType& index, set<int> remove_list) {
	int index_size = (int)index.size();

	for (int i = 0; i < index.size(); i++) {
		if (remove_list.find(index[i].second.id) != remove_list.end()) {
			index[i].first = INT_MAX;
		}
	}
	sort(index.begin(), index.end(), comp);
	IndexType::iterator it = find_if(index.begin(), index.end(),
		[](pair<unsigned int, MusicInfo> p){return p.first == INT_MAX; });
	index.resize(distance(index.begin(), it));
	return 0;
}

int Util::t_DeleteFingerDatabase(vector<vector<bitset<32>>>& finger_database,
	set<int> remove_list) {
	for (set<int>::iterator iter = remove_list.begin(); iter != remove_list.end(); iter++) {
		finger_database[*iter].clear();
	}
	return 0;
}

int Util::DeleteSomeIndex(string index_filepath, set<int> remove_list) {
	IndexType index;
	LoadIndex(index_filepath, index);
	int index_size = (int)index.size();

	for (int i = 0; i < index.size(); i++) {
		if (remove_list.find(index[i].second.id) != remove_list.end()) {
			index[i].first = INT_MAX;
		}
	}
	sort(index.begin(), index.end(), comp);
	IndexType::iterator it = find_if(index.begin(), index.end(),
		[](pair<unsigned int, MusicInfo> p){return p.first == INT_MAX; });
	index.resize(distance(index.begin(), it));
	OutputIndex(index_filepath, index);
	return 0;
}

int Util::DeleteSomeIndex(string index_filepath, string filepath) {
	ifstream fin(filepath, ios::in);
	set<int> remove_list;
	int songID;
	while (true) {
		fin >> songID;
		cout << songID << endl;
		remove_list.insert(songID);
		if (fin.eof())
			break;
	}
	cout << "set done" << endl;
	fin.close();
	DeleteSomeIndex(index_filepath, remove_list);
	return 0;
}

int Util::IncrementalBuildIndex(string index_filepath, string dir_path) {
	IndexType index;
	LoadIndex(index_filepath, index);
	cout << "load done!" << endl;
	vector<string> all_files = Util::load_dir(dir_path, "txt");

	for (int i = 0; i < all_files.size(); i++) {
		cout << i << endl;
		vector<unsigned int> audio_fingers;
		Util::load_one_file(all_files[i], audio_fingers);
		string originFile = all_files[i].substr(all_files[i].find_last_of("\\") + 1, all_files[i].find_last_of("."));
		int fileID = stoi(originFile);
		for (int j = 0; j < audio_fingers.size(); j++) {
			if (audio_fingers[j] == 0)
				continue;
			MusicInfo m(fileID, j);
			pair<unsigned int, MusicInfo> p(audio_fingers[j], m);
			index.push_back(p);
		}
	}
	sort(index.begin(), index.end(), comp);

	OutputIndex(index_filepath, index);
	cout << "Output done!" << endl;
	return 0;
}

int Util::IncrementalBuildFingerDatabase(string filepath_prefix, string dir_path) {
	LoadFingerDatabase(filepath_prefix);
	cout << "finger database load done!" << endl;
	vector<string> all_files = Util::load_dir(dir_path, "txt");

	for (size_t i = 0; i < all_files.size(); i++) {
		vector<unsigned int> audio_fingers;
		Util::load_one_file(all_files[i], audio_fingers);
		string originFile = all_files[i].substr(all_files[i].find_last_of("\\") + 1, all_files[i].find_last_of("."));
		int fileID = stoi(originFile);
		//这里要求fileID位置的音频已删除，或为空，否则会造成冲突或者finger_databse越界
		finger_database[fileID] = Util::VectorIntToVectorBitset(audio_fingers);
	}
	cout << "add done!" << endl;
	OutputFingerToFile(filepath_prefix);
	return 0;
}

int Util::_loadFingerFromOneFile(string filepath_prefix, unsigned int fileNum) {
	ifstream fin(filepath_prefix + to_string(fileNum), ios::in | ifstream::binary);
	int databaseSize = 0;
	fin.read(reinterpret_cast<char *>(&databaseSize), sizeof(databaseSize));
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

int Util::LoadFingerDatabase(string filepath_prefix) {
	finger_database.resize(DATABASE_SIZE);
	vector<thread> threads(OUTPUT_THREAD);
	for (int i = 0; i < OUTPUT_THREAD; i++) {
		threads[i] = thread(&Util::_loadFingerFromOneFile, filepath_prefix, i);
	}

	for (int i = 0; i < OUTPUT_THREAD; i++) {
		threads[i].join();
	}
	return 0;
}

int Util::_outputFingerToOneFile(string filepath_prefix, unsigned int fileNum) {
	ofstream fout(filepath_prefix + to_string(fileNum), ios::out | ofstream::binary);
	unsigned int databaseSize = (unsigned int)finger_database.size();
	fout.write(reinterpret_cast<char *>(&databaseSize), sizeof(unsigned int));
	unsigned int fingerSize = 0;
	vector<unsigned int> intVector;
	for (unsigned int i = 0; i < databaseSize; i++)	{
		if (i % OUTPUT_THREAD == fileNum && finger_database[i].size() != 0)	{
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

int Util::t_OutputFingerToFile(string filepath_prefix, vector<vector<bitset<32>>>& database) {
	finger_database.clear();
	finger_database = database;
	database.clear();
	OutputFingerToFile(filepath_prefix);
	return 0;
}

int Util::OutputFingerToFile(string filepath_prefix) {
	unsigned int databaseSize = (unsigned int)finger_database.size();
	vector<thread> threads(OUTPUT_THREAD);

	for (int i = 0; i < OUTPUT_THREAD; i++)	{
		threads[i] = thread(&Util::_outputFingerToOneFile, filepath_prefix, i);
	}

	for (int i = 0; i < OUTPUT_THREAD; i++)	{
		threads[i].join();
	}
	return 0;
}

int Util::DeleteFingerDatabase(string database_filepath, set<int> remove_list) {
	finger_database.resize(DATABASE_SIZE);
	LoadFingerDatabase(database_filepath);

	for (set<int>::iterator iter = remove_list.begin(); iter != remove_list.end(); iter++) {
		finger_database[*iter].clear();
	}
	OutputFingerToFile(database_filepath);
	finger_database.clear();
	return 0;
}

int Util::DeleteFingerDatabase(string database_filepath, string filepath) {
	ifstream fin(filepath, ios::in);
	set<int> remove_list;
	int songID;
	while (true) {
		fin >> songID;
		cout << songID << endl;
		remove_list.insert(songID);
		if (fin.eof())
			break;
	}
	fin.close();
	DeleteFingerDatabase(database_filepath, remove_list);
	return 0;
}

int Util::LoadIndex(string filepath, IndexType& index) {
	index.clear();
	int index_size = 0;
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

int Util::OutputIndex(string filepath, IndexType& index) {
	unsigned int index_size = (unsigned int)index.size();
	ofstream fout(filepath, ios::out | ofstream::binary);
	fout.write(reinterpret_cast<char *>(&index_size), sizeof(unsigned int));
	if (index_size == 0) {
		fout.close();
		return -1;
	}
	fout.write(reinterpret_cast<char *>(&index[0]), index.size() * sizeof(index[0]));
	fout.close();
	return 0;
}

vector<bitset<32>> Util::VectorIntToVectorBitset(vector<unsigned int> v) {
	vector<bitset<32>> bv;
	for (int i = 0; i < v.size(); i++) {
		bitset<32> b(v[i]);
		bv.push_back(b);
	}
	return bv;
}

vector<unsigned int> Util::VectorBitsetToVectorInt(vector<bitset<32>> v) {
	vector<unsigned int> iv;
	for (int i = 0; i < v.size(); i++) {
		unsigned int key = v[i].to_ulong();
		iv.push_back(key);
	}
	return iv;
}