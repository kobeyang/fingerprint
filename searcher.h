#include <vector>
#include <string>
#include <bitset>
#include "type.h"

extern int hit_index[500];
extern double duration_compare;
extern double duration_search;
extern double duration_not_find;

class Searcher {
private:
	FILE* fp;
	double diff;
	int match;
	std::vector<std::string> allFiles;
	int _insert_one_item(unsigned int key, MusicInfo& m);
	int _build_one_file_index(const std::string filepath);
	int _inner_search(unsigned int queryID, unsigned long item, FingerItem* finger_block, int block_size, int i, int& temp_dif);
	//void _do_statistics();
	bool _comp(std::pair<unsigned int, MusicInfo>, std::pair<unsigned int, MusicInfo>);
	long long _binary_search(unsigned int key);
	int _loadFingerFromOneFile(std::string filepath_prefix, unsigned int fileNum);
	int _outputFingerToOneFile(std::string filepath_prefix, unsigned int databaseSize, unsigned int fileNum);

public:
	IndexType index;
	std::vector<std::vector<std::bitset<32>>> finger_database;
	Searcher(){};
	int build_index(std::string dirPath);
	//int search(FingerItem* finger_block, int size, int& temp_diff);
	int SubSamplingSearch(unsigned int queryID, FingerItem* finger_block, int size, int& temp_diff);
	int compare_bitsets(int id, FingerItem* finger_block, int block_size, int i_frame_in_block, int i_frame_in_file);
	double get_mean_diff();
	int LoadIndex(std::string filepath);
	int LoadFingerDatabase(std::string filepath);
	int OutputIndexToFile(std::string filepath);
	int OutputFingerToFile(std::string filepath);
	int Clear();
};