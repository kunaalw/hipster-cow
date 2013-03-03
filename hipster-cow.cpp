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
#define NUM_CHROM_TIMES_TWO 8000000000 // #chromosomes to be read (x2 for Bitset) [Set larger than expected]
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

char decode (int pre, int post) {
	if (pre == 0 && post == 0)
		return 'A';
	else if (pre == 0 && post == 1)
		return 'C';
	else if (pre == 1 && post == 0)
		return 'G';
	else if (pre == 1 && post == 1)
		return 'T';
	else cerr << "Illegal character" << endl;
}

int find_repeats_thread_fun_num_lim (genome * reference_genome, int first, int last, int n) {

	ofstream outdata; //////////////////////////////////////////
	outdata.open("bleh.txt");
	int lengthCount = 0;
	int numRepeat = 0;
	char lastStringMatch[5]={'Z','Z','Z','Z','Z'};
	for (int i = first; i < last; i+=2) {
		if (i < (first + (2*n)));
		else {
			int countCharMatch = 0;
			char stringMatch[5]={'Z','Z','Z','Z','Z'};
			for (int j = 0; j < (2*n); j+=2) {
				int first = (*reference_genome)[i+j];
				int last = (*reference_genome)[i+j+1];
				int firstPre = (*reference_genome)[i+j-(2*n)];
				int lastPre = (*reference_genome)[i+j-(2*n)+1];
				
				char x = decode (first, last);
				char xPre = decode (firstPre, lastPre);
				
				if (x == xPre) {
					stringMatch[j/2] = xPre;
					countCharMatch++;
				}

				if (countCharMatch == n) {
					numRepeat++;
					i+=(2*(n-1));
					for (int p = 0; p <5; p++)
						lastStringMatch[p] = stringMatch[p];
					j=(2*n);
				}
				else {
					if ((numRepeat !=0) && (j == (2*n-2))) {
						outdata << "Match at position " << ((i/2)-(n*(numRepeat+1))) << " of length " << n;
						outdata << " with sequence ";
						int r = 0;
						while ((r < 5) && (lastStringMatch[r] != 'Z')) {
							outdata << lastStringMatch[r];
							r++;
						}
						outdata << " has been repeated " << (numRepeat+1) << " times." << endl;
						numRepeat = 0;
					}
				}
			}
		}
	}
	outdata.close(); //////////////////////////////////////////
	return 0;
}



int find_repeats_thread_fun (genome * reference_genome, int first, int last) {
					//cout << "here2" << endl; //*******************************************************************************
	vector<thread *> threadList;

	for (int i = 2; i <= 5 ; ++i)
		threadList.push_back(new thread(find_repeats_thread_fun_num_lim, reference_genome, first, last, i));
	
	for (int j = 0; j < 4; ++j) {
		threadList[j]->join();
		delete threadList[j];
	}

	return 0;
}


// **What does this function do? (TODO)
//   PARAM:  Genome type (i.e. bitset of size of genome) passed by reference (returned)
//   RETVAL: Fill this in later (TODO)
int find_repeats (genome * reference_genome, int sizeRef) {
	cout << "gets here 1234" << endl;

	vector<thread *> threadList;
	int sizeThreadGenome = ceil(sizeRef/NUM_THREADS);
	int lastInThread = -1;
	int firstInThread = -1;

	for (int i = 0; i < NUM_THREADS; ++i) {
				//cout << "here" << endl; //*******************************************************************************
		firstInThread = i*sizeThreadGenome;
		
		if (i == (NUM_THREADS-1))
			lastInThread = sizeRef-1;
		
		else lastInThread = (i+1)*sizeThreadGenome-1;
		cout << "first in thread: " << firstInThread << endl; //*******************************************************************************
		cout << "last in thread: " << lastInThread << endl; //*******************************************************************************
		cout << decode ((*reference_genome)[firstInThread], (*reference_genome)[firstInThread+1]) <<  decode ((*reference_genome)[firstInThread+2], (*reference_genome)[firstInThread+3]) << endl;
		threadList.push_back(new thread(find_repeats_thread_fun, reference_genome, firstInThread, lastInThread));
		cout << "yoyo" << endl;
	}
	
	for (int j = 0; j < NUM_THREADS; ++j) {
		threadList[j]->join();
		delete threadList[j];
	}
	return 0;
}

int outputTable (standrep tableRet[5][5][5][5][5]) {

	cout << "|  Pattern  |" << "|      Start      |" << "|       End       |" << endl;

	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					for (int m = 0; m < 5; m++) {
						if (tableRet[i][j][k][l][m].size() == 0) continue;
						else {
							for(unsigned int n = 0; n < tableRet[i][j][k][l][m].size(); n++) {
								cout << "Pattern: " << tableRet[i][j][k][l][m][n].pattern << "     " << tableRet[i][j][k][l][m][n].startPos << "     " << tableRet[i][j][k][l][m][n].endPos << endl;
								delete &(tableRet[i][j][k][l][m][n]);
								tableRet[i][j][k][l][m].erase(tableRet[i][j][k][l][m].begin()+1);
							}
						}
					}
				}
			}
		}
	}
	return 0;

}


// **Main function
int main (int argc, char *argv[]) {

		cout << "gets here 1234" << endl;
	genome refGenome;
	int sizeRef = 0;
		cout << "gets here 1234" << endl;
	if(!(sizeRef = input_ref_genome(refGenome)))
		cerr << "Cannot open input file" << endl;
		cout << "gets here 1234" << endl;
	vector<string> retReads;
	input_target_reads (retReads);
	cout << "Size of genome is: " <<sizeRef << endl << endl;
	find_repeats(&refGenome,sizeRef);

	standrep refRepTable[5][5][5][5][5];

	/*
	ofstream outdata; // outdata is like cin
   int j; // loop index
   //int num[5] = {4, 3, 6, 7, 12}; // list of output values
  outdata.open("bleh.txt"); // opens the file
   if( !outdata ) { // file couldn't be opened
      cerr << "Error: file could not be opened" << endl;
      exit(1);
   }
  for (int i = 0; i < sizeRef; i+=2)
  outdata << decode(refGenome[i],refGenome[i+1]);
   outdata.close();*/

}

/*
GAAAGCAGCAGCAGCTCAAATACGGTCATAGTACTCAAACTACTACTACTACTACTACAAAATTATACCATTACGTACGTACGGATCACTAAAGCAAATAACAGATGACTGTAGGTTAGTCAAGATATAAAACAGGCCGAAGACTTGGTTTTGCCCCATAAACAATCAATCCAGTGCTCGCTCGAAACGGAAAGAAAATCAGAGAGGTCGGAAACAGATACAATCGTGGCGGTGGCAGCAGTAGTTAGTTGACGCGGAATGCATCAAGATATATATATATATATATATATATATATATATATATATATATATGGAATAATTTCTTGGCCTCTGCGGCAGCCACTGGCAACCCACATCAAGTTGTTAAGGAAGGTATAACGGTCATCTCCAGTTTCCATGGAGAGGTTAACGCGGTATTTATATGAGTTGGCAGAGGAGGAGGAGGAGTTAATCCCATCCGCAAGTTGAGGGTCACTACTATTGAGCTTATCCAGCACATCTTTCGCGCCTAGCTACGTACGTACGTACGTACGTGTCAACCAC
*/