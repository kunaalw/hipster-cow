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
#include <mutex>
#include <functional>
#include <chrono>

#define TARGET_GENOME_IN "gen_wip_target.txt" // Path of file with the reference genome
#define NUM_CHROM_TIMES_TWO 7500000000ul // #chromosomes to be read (x2 for Bitset) [Set larger than expected]
#define NUM_THREADS 1 // #threads spawned by repeat finder (reference)
#define TARGET_READFILE "reads.txt"
#define READ_LENGTH 30
#define READ_SIZE_MULTIPILER 3

using namespace std; 

//typedef bitset<NUM_CHROM_TIMES_TWO> genome;
typedef bitset<NUM_CHROM_TIMES_TWO> genome;

typedef struct STRInstance {
    __int64 startPos;
	int numTimesRep;
	string pattern;
	int lengthPattern;
} standrepinst;

typedef vector<standrepinst> standrep;

mutex vecSync;


// **Inputs the reference genome into a string
//   PARAM:  Genome type (i.e. bitset of size of genome) passed by reference (returned)
//   RETVAL: integer which returns size of genome when successful, -1 otherwise
__int64 input_ref_genome (genome * reference_genome) {
	FILE *file = fopen(TARGET_GENOME_IN, "r");
	__int64 i = 0;
	int j = 0;
    size_t n = 0;
    char c;
    if (file == NULL)
        return -1; //could not open file
    while ((c = fgetc(file)) != EOF) {
		i++;
		j++;
		if (c == 'A') {
			reference_genome->set(n++,0);
			reference_genome->set(n++,0);
		}
		else if (c == 'C') {
			reference_genome->set(n++,0);
			reference_genome->set(n++,1);
		}
		else if (c == 'G') {
			reference_genome->set(n++,1);
			reference_genome->set(n++,0);
		}
		else if (c == 'T') {
			reference_genome->set(n++,1);
			reference_genome->set(n++,1);
		}
		else {
			cerr << "There is an error in the input file - invalid character" << endl;
			return -1;
		}
    }
	return i;
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



int make_reads_seg (genome * reference_genome, __int64 first, __int64 last) {
	__int64 j;
	__int64 length_seg = abs(last-first) + 1;
	__int64 num_reads_seg = ceil(((length_seg/READ_LENGTH)*READ_SIZE_MULTIPILER));

	ofstream outdata;
	outdata.open (TARGET_READFILE, fstream::out | fstream::app);
	if( !outdata ) {
		cerr << "Error: file could not be opened" << endl;
		exit(1);
	}
	for (j=0; j<num_reads_seg; ++j) {
		__int64 i = rand()%(length_seg - READ_LENGTH + 1);
		__int64 start_pos_read = 2*(first + i);
		__int64 k;
		string tempStore = "";

		for ( k=0; k<(READ_LENGTH*2); k+=2)
			tempStore = tempStore + decode((*reference_genome)[start_pos_read+k], (*reference_genome)[start_pos_read+k+1]);
		tempStore = tempStore + "\n";
		vecSync.lock();
		//outdata << endl << "FIRST IS: " << first << endl;
		///outdata << "LAST IS: " << last << endl;
		outdata << tempStore;
		//outdata << endl << "FIRST AGAIN... IS: " << first << " and read is from " << (first + i) << " to " << (first + i + READ_LENGTH) << endl << endl;
		vecSync.unlock();
	}

   return 0;
}



int make_reads (genome * reference_genome, __int64 sizeRef) {
	
	vector<thread *> threadList;
	__int64 sizeThreadGenome = ceil(sizeRef/NUM_THREADS);
	__int64 lastInThread = -1;
	__int64 firstInThread = -1;

	for (int i = 0; i < NUM_THREADS; ++i) {
		firstInThread = i*sizeThreadGenome;
		
		if (i == (NUM_THREADS-1))
			lastInThread = sizeRef-1;
		
		else lastInThread = (i+1)*sizeThreadGenome-1;
		threadList.push_back(new thread(make_reads_seg, reference_genome, firstInThread, lastInThread));
	}
	
	for (int j = 0; j < NUM_THREADS; ++j) {
		threadList[j]->join();
		delete threadList[j];
	}
	
	return 0;
}




// **Main function
int main (int argc, char *argv[]) {

	genome *targetGenome = new genome;
	__int64 sizeRef = 0;
	if(!(sizeRef = input_ref_genome(targetGenome)))
		cerr << "Cannot open input file" << endl;
	cout << "STATUS: Target genome input complete" << endl;

	//for (int i = 1508881; i <= 1508911; i++)
		//cout << decode((*targetGenome)[2*i], (*targetGenome)[2*i+1]);
	//cout << endl;
	ofstream outdata;
	outdata.open ("yolo.txt", fstream::out | fstream::app);


	make_reads(targetGenome,sizeRef);
	cout << "STATUS: All tasks completed successfully ---" << endl << endl;
	
	delete(targetGenome);
}

/*
GAAAGCAGCAGCAGCTCAAATACGGTCATAGTACTCAAACTACTACTACTACTACTACAAAATTATACCATTACGTACGTACGGATCACTAAAGCAAATAACAGATGACTGTAGGTTAGTCAAGATATAAAACAGGCCGAAGACTTGGTTTTGCCCCATAAACAATCAATCCAGTGCTCGCTCGAAACGGAAAGAAAATCAGAGAGGTCGGAAACAGATACAATCGTGGCGGTGGCAGCAGTAGTTAGTTGACGCGGAATGCATCAAGATATATATATATATATATATATATATATATATATATATATATATGGAATAATTTCTTGGCCTCTGCGGCAGCCACTGGCAACCCACATCAAGTTGTTAAGGAAGGTATAACGGTCATCTCCAGTTTCCATGGAGAGGTTAACGCGGTATTTATATGAGTTGGCAGAGGAGGAGGAGGAGTTAATCCCATCCGCAAGTTGAGGGTCACTACTATTGAGCTTATCCAGCACATCTTTCGCGCCTAGCTACGTACGTACGTACGTACGTGTCAACCAC
*/