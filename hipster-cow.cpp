// C++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>

#define REF_GENOME_IN "ref_genome.txt" // Path of file with the reference genome
#define READ_TARGET "read_target.txt" // Path of file with the reads

using namespace std;


// **Inputs the reference genome into a string
//   PARAM:  string passed by reference (returned)
//   RETVAL: integer which returns 1 when successful, 0 otherwise
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


// **Inputs the target reads into a string vector
//   PARAM:  string vector passed by reference (returned with reads)
//   RETVAL: number of reads (i.e. size of vector reads)
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


// **Main function
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