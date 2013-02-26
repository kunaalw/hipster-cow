// C++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

#define REF_GENOME_IN "ref_genome.txt" // The file with the reference genome
#define READ_TARGET "read_target.txt" // The file with the reads

using namespace std;

int input_ref_genome (string *buff) {
	string line;
	ifstream infile_ref_genome (REF_GENOME_IN);
	if (infile_ref_genome.is_open())
	{
		while (infile_ref_genome.good())
		{
			getline (infile_ref_genome, line);
			*buff = *buff + line + "\n";
		}
		infile_ref_genome.close();
		return 1;
	}
	return 0;
}

int input_target_reads (vector<string>& reads) {

	
	int numReads = 0;
	string line;
	
	ifstream myfile (READ_TARGET);
	if (myfile.is_open())
	{
		while ( myfile.good())
		{
			getline (myfile, line);
			reads.push_back(line);
			numReads++;
		}
		myfile.close();
		return numReads;
	}
	return -1;
}

int main (int argc, char *argv[]) {
	string refGenome;
	if(!input_ref_genome(&refGenome))
		cerr << "Cannot open input file" << endl;
	
	vector<string> retReads;
	input_target_reads (retReads);
	cout << "First line is " << retReads[0] << endl;
	cout << "Third line is " << retReads[2] << endl;
	return 0;
}