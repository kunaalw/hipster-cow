#define _CRT_SECURE_NO_DEPRECATE // To allow for deprecated function fopen to be used

// C++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <stdio.h>

#define REF_GENOME_IN "ref_genome.txt" // Path of file with the reference genome
#define READ_TARGET "read_target.txt" // Path of file with the reads
#define NUM_CHROM 1200 // Number of chromosomes to be read

using namespace std;


// **Inputs the reference genome into a string
//   PARAM:  string passed by reference (returned)
//   RETVAL: integer which returns 1 when successful, 0 otherwise
int input_ref_genome (string *buff) {
	//string line;
	char inputChar;
	size_t bitSetSize = NUM_CHROM*2;
	bitset<NUM_CHROM>mybits;
	//ifstream infile_ref_genome (REF_GENOME_IN);
	/*
	if (infile_ref_genome.is_open())
	{
		while (infile_ref_genome.good())
		{
			//getline (infile_ref_genome, line);
			//*buff = *buff + line + "\n";
			inputChar = getc (infile_ref_genome);
			//if (inputChar == 'A') 
		}
		infile_ref_genome.close();
		return 1;
	}*/

	//*********************
	FILE *file = fopen(REF_GENOME_IN, "r");
    //char *code;
    size_t n = 0;
    char c;

    if (file == NULL)
        return NULL; //could not open file

    //code = malloc(1000);

    while ((c = fgetc(file)) != EOF) {
		cout << "checkpoint 1" << endl;
		cout << "char c is " << c << endl;
		if (c == 'A') {
			mybits.set(n++,0);
			mybits.set(n++,0);
		}
		else if (c == 'C') {
			mybits.set(n++,0);
			mybits.set(n++,1);
		}
		else if (c == 'G') {
			mybits.set(n++,1);
			mybits.set(n++,0);
		}
		else if (c == 'T') {
			mybits.set(n++,1);
			mybits.set(n++,1);
		}
		else cerr << "There is an error in the input file - invalid character" << endl;
        //mybits.set(n++,0);//code[n++] = (char) c;
    }
	cout << "mybits[0] = " << mybits[0] << endl;
	cout << "mybits[1] = " << mybits[1] << endl;// << "mybits[2] = " mybits[2] << endl;
	cout << "mybits[2] = " << mybits[2] << endl;
	cout << "mybits[3] = " << mybits[3] << endl;
	cout << "mybits[4] = " << mybits[4] << endl;
	cout << "mybits[5] = " << mybits[5] << endl;
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
	cout << "The reference genome is " << endl << refGenome << endl << endl;
	cout << "First line is " << retReads[0] << endl;
	cout << "Third line is " << retReads[2] << endl;

	bitset<33> third (string("01010101010101110101010010001010111101010101010110001010101010101010001010101010111001010101010101001010101001010010101001010010100101010101001010001010011110010100101011"));
	cout << "third is " << third << endl;
	cout << "size of third is " << sizeof(third) << endl;
	cout << third[0] << endl;
	cout << third[1] << endl;
	cout << third[2] << endl;
	cout << third[3] << endl;
	cout << third[4] << endl;

	return 0;
}