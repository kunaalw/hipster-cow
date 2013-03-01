#define _CRT_SECURE_NO_DEPRECATE // To allow for deprecated function fopen to be used

// C++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <stdio.h>
#include <thread>

#define REF_GENOME_IN "ref_genome.txt" // Path of file with the reference genome
#define READ_TARGET "read_target.txt" // Path of file with the reads
#define NUM_CHROM_TIMES_TWO 1200 // #chromosomes to be read (x2 for Bitset) [Set larger than expected]
#define NUM_THREADS 10 // #threads spawned by repeat finder (reference)

using namespace std;

typedef bitset<NUM_CHROM_TIMES_TWO> genome;

// **Inputs the reference genome into a string
//   PARAM:  Genome type (i.e. bitset of size of genome) passed by reference (returned)
//   RETVAL: integer which returns size of genome when successful, -1 otherwise
int input_ref_genome (genome & reference_genome) {
	FILE *file = fopen(REF_GENOME_IN, "r");

    size_t n = 0;
    char c;

    if (file == NULL)
        return -1; //could not open file

    while ((c = fgetc(file)) != EOF) {
		cout << "checkpoint 1" << endl;
		cout << "char c is " << c << endl;
		if (c == 'A') {
			reference_genome.set(n++,0);
			reference_genome.set(n++,0);
		}
		else if (c == 'C') {
			reference_genome.set(n++,0);
			reference_genome.set(n++,1);
		}
		else if (c == 'G') {
			reference_genome.set(n++,1);
			reference_genome.set(n++,0);
		}
		else if (c == 'T') {
			reference_genome.set(n++,1);
			reference_genome.set(n++,1);
		}
		else {
			cerr << "There is an error in the input file - invalid character" << endl;
			return -1;
		}
    }
	return n;
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

//int find_repeats_thread_fun(


// **What does this function do? (TODO)
//   PARAM:  Genome type (i.e. bitset of size of genome) passed by reference (returned)
//   RETVAL: Fill this in later (TODO)
int find_repeats (genome & reference_genome) {
	vector<thread *> threadList;
	for (int i = 0; i < NUM_THREADS; ++i)
    //threadList.push_back(new thread(workerFunc));

  for (int j = 0; j < NUM_THREADS; ++j)
  {
      threadList[j]->join();
      delete threadList[j];
  }
	return 0;
}


// **Main function
int main (int argc, char *argv[]) {
	genome refGenome;
	int sizeRef = 0;
	if(!(sizeRef = input_ref_genome(refGenome)))
		cerr << "Cannot open input file" << endl;
	
	vector<string> retReads;
	input_target_reads (retReads);
	cout << "sizeRef is: " <<sizeRef << endl;
	cout << "The reference genome is " << endl;
	for (int i = 0; i < sizeRef; i++)
	{
		cout << refGenome[i];
	}
	cout << "First line is " << retReads[0] << endl;
	cout << "Third line is " << retReads[2] << endl;

	return 0;
}