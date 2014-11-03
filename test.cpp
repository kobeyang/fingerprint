#include <iostream>
#include <bitset>
#include "util.h"

using namespace std;

/*
void find_diff()
{
vector<bitset<32>> fingers_192, fingers_96;
vector<string> all_files_192, all_files_96;
string dirpath_192 = "E:\\FingerprintingExtraction\\find_diff_position\\192wave3min\\fingers";
string dirpath_96 = "E:\\FingerprintingExtraction\\find_diff_position\\96wave3min\\fingers";
all_files_192 = Util::load_dir(dirpath_192,"txt");
all_files_96 = Util::load_dir(dirpath_96,"txt");
int diff_bit[32];
for(int i = 0; i < 32; i++)
diff_bit[i] = 0;
for(int k = 0; k < (signed)all_files_192.size(); k++)
{
Util::load_one_file(dirpath_192 + "\\" +all_files_192[k], fingers_192);
Util::load_one_file(dirpath_96 + "\\" +all_files_96[k], fingers_96);
cout<<k<<endl;
for(int i = 0; i < (signed)fingers_192.size(); i++)
{
for(int j = 0; j < 32; j++)
{
if(fingers_192[i].at(j) != fingers_96[i].at(j))
{
diff_bit[j]++;
}
}
}
}
int index_bit[32];
for(int i = 0; i < 32; i++)
index_bit[i] = i;

for(int i = 0; i < 31; i++)
for(int j = i + 1; j < 32; j++)
{
if(diff_bit[i] < diff_bit[j])
{
int temp = diff_bit[i];
diff_bit[i] = diff_bit[j];
diff_bit[j] = temp;
temp = index_bit[i];
index_bit[i] = index_bit[j];
index_bit[j] = temp;
}
}
for(int i = 0; i < 32; i++)
{
cout<<index_bit[i]<<" "<<diff_bit[i]<<endl;
}
}
*/