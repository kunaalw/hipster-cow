// C++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

#define REF_GENOME_IN "ref_genome.txt" // The file with the reference genome

using namespace std;

int input_ref_genome(string *buff) {
	string line;
	ifstream myfile (REF_GENOME_IN);
	if (myfile.is_open())
	{
		while ( myfile.good())
		{
			getline (myfile, line);
			*buff = *buff + line + "\n";
		}
		myfile.close();
		return 1;
	}
	return 0;
}



int main (int argc, char *argv[]) {
	string refGenome ="";
	if(!input_ref_genome(&refGenome))
		cerr << "Cannot open input file" << endl;
	return 0;
}