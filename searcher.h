#include <bitset>
#include <string>
#include <map>
#include <vector>
#include "type.h"

extern int hit_index[500];
extern double duration_compare;
extern double duration_search;
extern double duration_not_find;

class Searcher {
private:
	std::vector<std::string> _allFiles;
	int _InsertOneItem(unsigned int key, MusicInfo& m);
	int _BuildOneFileIndex(const std::string filepath);
	int _InnerSearch(unsigned long item, FingerItem* finger_block,
		int block_size, int i, std::map<int, int>*);
	int _LoadFingerFromOneFile(std::string filepath_prefix, unsigned int fileNum);
	int _OutputFingerToOneFile(std::string filepath_prefix, unsigned int databaseSize, unsigned int fileNum);
	long long _BinarySearch(unsigned int key);
	double _CompareBitsets(int id, FingerItem* finger_block, int block_size,
		int i_frame_in_block, int i_frame_in_file);

public:
	Searcher(){};
	IndexType _index;
	std::vector<std::vector<std::bitset<32>>> _finger_database;
	int BuildIndex(std::string dirPath);
	int Search(FingerItem* finger_block, int size, int& temp_diff);
	int LoadIndex(std::string filepath);
	int LoadFingerDatabase(std::string filepath);
	int OutputIndexToFile(std::string filepath);
	int OutputFingerToFile(std::string filepath);
	int Clear();
	void DoStatistics();
};