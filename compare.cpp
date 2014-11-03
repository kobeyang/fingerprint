#include <iostream>
#include <fstream>

using namespace std;

void readFile(char* file, int finger[256][32]) {
	FILE *fp = fopen(file, "r");
	for (int i = 0; i < 256; i++) {
		for (int j = 0; j < 32; j++) {
			fscanf(fp, "%d", &finger[i][j]);
		}
	}
	return;
}

int compare(char* file1, char* file2) {
	int finger1[256][32];
	int finger2[256][32];
	readFile(file1, finger1);
	readFile(file2, finger2);
	int numOfError = 0;
	for (int i = 0; i < 256; i++)
	for (int j = 0; j < 32; j++)
	if (finger1[i][j] != finger2[i][j])
		numOfError++;
	return numOfError;
}

/*
int main(int argc, char* argv[])
{
if(argc < 3)
{
cout<<"Usage: ./compare finger1 finger2"<<endl;
return -1;
}
int numOfError = compare(argv[1], argv[2]);
cout<<numOfError<<endl;
}
*/