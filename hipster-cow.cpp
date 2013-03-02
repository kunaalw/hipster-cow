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
#include "reference_STR_tree.h"

#define REF_GENOME_IN "ref_genome.txt" // Path of file with the reference genome
#define READ_TARGET "read_target.txt" // Path of file with the reads
#define NUM_CHROM_TIMES_TWO 1200 // #chromosomes to be read (x2 for Bitset) [Set larger than expected]
#define NUM_THREADS 10 // #threads spawned by repeat finder (reference)

using namespace std;


typedef bitset<NUM_CHROM_TIMES_TWO> genome;

typedef struct STRInstance {
    int startPos;
	int endPos;
	string pattern;
	int lengthPattern;
} standrepinst;

typedef vector<standrepinst> standrep;


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

int find_repeats_thread_fun_num_lim(genome & reference_genome, int first, int last, int n) {
	return 0;
}



int find_repeats_thread_fun(genome & reference_genome, int first, int last) {

	vector<thread *> threadList;

	for (int i = 2; i <= 5 ; ++i)
		threadList.push_back(new thread(find_repeats_thread_fun_num_lim, reference_genome, first, last, i));
	
	for (int j = 2; j <= 5; ++j) {
		//threadList[j]->join();
		//delete threadList[j];
	}

	return 0;
}


// **What does this function do? (TODO)
//   PARAM:  Genome type (i.e. bitset of size of genome) passed by reference (returned)
//   RETVAL: Fill this in later (TODO)
int find_repeats (genome & reference_genome, int sizeRef) {
	vector<thread *> threadList;
	int sizeThreadGenome = ceil(sizeRef/NUM_THREADS);
	int lastInThread = -1;
	int firstInThread = -1;

	for (int i = 0; i < NUM_THREADS; ++i) {
		firstInThread = i*sizeThreadGenome;
		
		if (i == (NUM_THREADS-1))
			lastInThread = sizeRef-1;
		
		else lastInThread = (i+1)*sizeThreadGenome-1;

		threadList.push_back(new thread(find_repeats_thread_fun, reference_genome, firstInThread, lastInThread));
	}
	
	for (int j = 0; j < NUM_THREADS; ++j) {
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
	cout << "Size of genome is: " <<sizeRef << endl << endl;
	find_repeats(refGenome,sizeRef);

	standrep refRepTable[5][5][5][5][5];
	
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					for (int m = 0; m < 5; m++) {
						for(unsigned int n = 0; n < refRepTable[i][j][k][l][m].size(); n++)
						{
							delete &(refRepTable[i][j][k][l][m][n]);
							refRepTable[i][j][k][l][m].erase(refRepTable[i][j][k][l][m].begin()+1);
						}
					}
				}
			}
		}
	}

}