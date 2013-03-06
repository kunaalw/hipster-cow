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
#include "reference_STR_tree.h"

#define REF_GENOME_IN "gen_wip.txt" // Path of file with the reference genome
#define READ_TARGET "reads_wip.txt" // Path of file with the reads
#define NUM_CHROM_TIMES_TWO 7500000000ul // #chromosomes to be read (x2 for Bitset) [Set larger than expected]
#define NUM_THREADS 10 // #threads spawned by repeat finder (reference)
#define REF_OUTPUT "ref_output_wip.txt"
#define MIN_REP 5
#define MIN_READ_REP 4
#define READ_LENGTH 30

using namespace std; 

//typedef bitset<NUM_CHROM_TIMES_TWO> genome;
typedef bitset<NUM_CHROM_TIMES_TWO> genome;

typedef struct STRInstance {
    __int64 startPos;
	int numTimesRep;
	string pattern;
	int lengthPattern;
	int numTimesRepTarget;
	int status;
} standrepinst;

typedef vector<standrepinst> standrep;

mutex vecSync;
mutex endSync;


// **Inputs the reference genome into a string
//   PARAM:  Genome type (i.e. bitset of size of genome) passed by reference (returned)
//   RETVAL: integer which returns size of genome when successful, -1 otherwise
__int64 input_ref_genome (genome * reference_genome) {
	FILE *file = fopen(REF_GENOME_IN, "r");
	__int64 i = 0;
    size_t n = 0;
    char c;
    if (file == NULL)
        return -1; //could not open file
    while ((c = fgetc(file)) != EOF) {
		i++;
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

int find_repeats_thread_fun_num_lim (genome * reference_genome, __int64 first, __int64 last, int n, standrep tableRet[5][5][5][5][5]) {
	cout << "STATUS: Length-seperate thread formed (n is: " << n << ")" << endl;
	ofstream outdata;
	int lengthCount = 0;
	int numRepeat = 0;
	char lastStringMatch[5]={'Z','Z','Z','Z','Z'};
	for (__int64 i = (2*first); i < (2*last); i+=2) {
		if (i < ((2*first) + (2*n)));
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

				int checkRepFlag = 0;
				if (((n == 4) && (lastStringMatch[0] == lastStringMatch[2]) && (lastStringMatch[1] == lastStringMatch[3]))
					|| ((n == 3) && (lastStringMatch[0] == lastStringMatch[1]) && (lastStringMatch[0] == lastStringMatch[2]))
					  || ((n== 5) && (lastStringMatch[0] == lastStringMatch[1]) || (lastStringMatch[0] == lastStringMatch[2]) 
					    || (lastStringMatch[0] == lastStringMatch[3]) || (lastStringMatch[0] == lastStringMatch[4]))) {
					checkRepFlag=1;
				}

				if (countCharMatch == n) {
					numRepeat++;
					i+=(2*(n-1));
					for (int p = 0; p <5; p++)
						lastStringMatch[p] = stringMatch[p];
					j=(2*n);
				}
				else {
					if ((numRepeat >= (MIN_REP-1)) && (j == (2*n-2))) {
						int lim[5] = {4,4,4,4,4};
						int r = 0;		
						standrepinst newRep;
						newRep.pattern = "";

						while ((r < 5) && (lastStringMatch[r] != 'Z')) {
							if (lastStringMatch[r] == 'A') lim[r] = 0;
							if (lastStringMatch[r] == 'C') lim[r] = 1;
							if (lastStringMatch[r] == 'G') lim[r] = 2;
							if (lastStringMatch[r] == 'T') lim[r] = 3;
							newRep.pattern = newRep.pattern+lastStringMatch[r];
							r++;
						}

						newRep.startPos = (i/2)-(n*(numRepeat+1));
						newRep.lengthPattern = n;
						newRep.numTimesRep = (numRepeat+1);
						newRep.numTimesRepTarget = 0;
						newRep.status = 0;

						__int64 i = tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]].size();
						
						vecSync.lock();
						tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]].push_back(newRep);
						vecSync.unlock();
						numRepeat = 0;
					}
					else if ((numRepeat !=0) && (j == (2*n-2))) numRepeat = 0;

					if (checkRepFlag == 0) {
						if ((numRepeat !=0) && (j == (2*n-2))) {
							int lim[5] = {4,4,4,4,4};
							int r = 0;		
							standrepinst newRep;
							newRep.pattern = "";

							while ((r < 5) && (lastStringMatch[r] != 'Z')) {
								if (lastStringMatch[r] == 'A') lim[r] = 0;
								if (lastStringMatch[r] == 'C') lim[r] = 1;
								if (lastStringMatch[r] == 'G') lim[r] = 2;
								if (lastStringMatch[r] == 'T') lim[r] = 3;
								newRep.pattern = newRep.pattern+lastStringMatch[r];
								r++;
							}

							newRep.startPos = (i/2)-(n*(numRepeat+1));
							newRep.lengthPattern = n;
							newRep.numTimesRep = (numRepeat+1);
							tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]].push_back(newRep);
							numRepeat = 0;
						}
					}
					else numRepeat = 0;
				}
			}
		}
	}
	return 0;
}



int find_repeats_thread_fun (genome * reference_genome, __int64 first, __int64 last, standrep tableRet[5][5][5][5][5]) {

	cout << "STATUS: Split thread formed (starts at: " << first << ")" << endl;
	vector<thread *> threadList;

	for (int i = 2; i <= 5 ; ++i)
		threadList.push_back(new thread(find_repeats_thread_fun_num_lim, reference_genome, first, last, i, tableRet));
	
	for (int j = 0; j < 4; ++j) {
		threadList[j]->join();
		delete threadList[j];
	}

	return 0;
}


// **What does this function do? (TODO)
//   PARAM:  Genome type (i.e. bitset of size of genome) passed by reference (returned)
//   RETVAL: Fill this in later (TODO)
int find_repeats (genome * reference_genome, __int64 sizeRef, standrep tableRet[5][5][5][5][5]) {

	vector<thread *> threadList;
	__int64 sizeThreadGenome = ceil(sizeRef/NUM_THREADS);
	__int64 lastInThread = -1;
	__int64 firstInThread = -1;

	for (int i = 0; i < NUM_THREADS; ++i) {
		firstInThread = i*sizeThreadGenome;
		
		if (i == (NUM_THREADS-1))
			lastInThread = sizeRef-1;
		
		else lastInThread = (i+1)*sizeThreadGenome-1;
		cout << decode ((*reference_genome)[firstInThread], (*reference_genome)[firstInThread+1]) <<  decode ((*reference_genome)[firstInThread+2], (*reference_genome)[firstInThread+3]) << endl;
		threadList.push_back(new thread(find_repeats_thread_fun, reference_genome, firstInThread, lastInThread, tableRet));
	}
	
	for (int j = 0; j < NUM_THREADS; ++j) {
		threadList[j]->join();
		delete threadList[j];
	}
	return 0;
}

int outputTable (standrep tableRet[5][5][5][5][5]) {

	cout << "\n\nPattern, " << "Length, " << "Start, " << "End \n" << endl;
	for (int i = 0; i < 5; i++) {
		for (int j = 0; j < 5; j++) {
			for (int k = 0; k < 5; k++) {
				for (int l = 0; l < 5; l++) {
					for (int m = 0; m < 5; m++) {
						if (tableRet[i][j][k][l][m].size() == 0);
						else {
							for(__int64 n = 0; n < tableRet[i][j][k][l][m].size(); n++)
								cout << tableRet[i][j][k][l][m][n].pattern << ", " << tableRet[i][j][k][l][m][n].lengthPattern << ", " << tableRet[i][j][k][l][m][n].startPos << ", " << tableRet[i][j][k][l][m][n].numTimesRep << endl;
							tableRet[i][j][k][l][m].clear();
						}
					}
				}
			}
		}
	}
	return 0;

}

//READS FUNCTIONS

int find_repeat_single_read_num_lim(string line, int n, standrep tableRet[5][5][5][5][5]) {
	int lengthCount = 0;
	int numRepeat = 0;
	for (int i = 0; i <= READ_LENGTH-n; i++) {
		if (i < n);
		else {
			int countCharMatch = 0;
			for (int j = 0; j < n; j++) {
				if (line[i+j] == line[i+j-n])
					countCharMatch++;

				if (countCharMatch == n) {
					numRepeat++;
					i+=(n-1);
					//for (int p = 0; p <5; p++)
						//lastStringMatch[p] = stringMatch[p];
					j=n;
				}
				else if ((numRepeat >= (MIN_READ_REP-1)) && (j == (n-1))) {
					endSync.lock();
					cout << "The pattern ";
					for (int k = 0; k < n; k++)
						cout << line[i-n+k];
					cout << " has been repeated " << numRepeat+1 << " times in line " << line << endl;
					numRepeat = 0;
					endSync.unlock();
				}
				else if ((numRepeat != 0) && (j == (n-1))) {
					numRepeat = 0;
				}
			}
		}
	}
	return 0;
}



int find_repeats_single_read(string line, standrep tableRet[5][5][5][5][5]) {
	vector<thread *> threadList;

	for (int i = 2; i <= 5 ; ++i)
		threadList.push_back(new thread(find_repeat_single_read_num_lim, line, i, tableRet));
	
	for (int j = 0; j < 4; ++j) {
		threadList[j]->join();
		delete threadList[j];
	}
	return 0;
}

// **Inputs the target reads into a string vector
//   PARAM:  string vector passed by reference (returned with reads)
//   RETVAL: number of reads (i.e. size of vector reads)
int input_target_reads (standrep tableRet[5][5][5][5][5]) {
	string line[NUM_THREADS];
	__int64 counter = 0;
	ifstream myfile (READ_TARGET);
	if (myfile.is_open()) {
		while ( myfile.good()) {
			vector<thread *> threadList;
			for (int i = 0; i < NUM_THREADS; i++) {
				getline(myfile, line[i], '\n');
				threadList.push_back(new thread(find_repeats_single_read, line[i], tableRet));
				counter +=10;
				if (counter%10000 == 0) cout << "STATUS: Processed line #" << counter << endl;
			}

			for (int j = 0; j < NUM_THREADS; j++) {
				threadList[j]->join();
				delete threadList[j];
			}
		}
		myfile.close();
	}
	return -1;
}


// **Main function
int main (int argc, char *argv[]) {

	genome *refGenome = new genome;
	__int64 sizeRef = 0;
	if(!(sizeRef = input_ref_genome(refGenome)))
		cerr << "Cannot open input file" << endl;
	cout << "STATUS: Reference genome input complete" << endl;

	vector<string> retReads;
	cout << "STATUS: Target reads input complete" << endl;

	cout << "INFO: Size of genome is: " <<sizeRef << endl << endl;
	
	standrep refRepTable[5][5][5][5][5];
	find_repeats(refGenome,sizeRef, refRepTable);
	cout << "STATUS: All STRs in Reference located" << endl;
	input_target_reads (refRepTable);

	cout << "STATUS: All tasks completed successfully ---" << endl << endl;
	outputTable(refRepTable);
	delete(refGenome);

}

/*
GAAAGCAGCAGCAGCTCAAATACGGTCATAGTACTCAAACTACTACTACTACTACTACAAAATTATACCATTACGTACGTACGGATCACTAAAGCAAATAACAGATGACTGTAGGTTAGTCAAGATATAAAACAGGCCGAAGACTTGGTTTTGCCCCATAAACAATCAATCCAGTGCTCGCTCGAAACGGAAAGAAAATCAGAGAGGTCGGAAACAGATACAATCGTGGCGGTGGCAGCAGTAGTTAGTTGACGCGGAATGCATCAAGATATATATATATATATATATATATATATATATATATATATATATGGAATAATTTCTTGGCCTCTGCGGCAGCCACTGGCAACCCACATCAAGTTGTTAAGGAAGGTATAACGGTCATCTCCAGTTTCCATGGAGAGGTTAACGCGGTATTTATATGAGTTGGCAGAGGAGGAGGAGGAGTTAATCCCATCCGCAAGTTGAGGGTCACTACTATTGAGCTTATCCAGCACATCTTTCGCGCCTAGCTACGTACGTACGTACGTACGTGTCAACCAC
*/