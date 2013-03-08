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
#define MIN_REP 4
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


/*
 __int64 startPos;
	int numTimesRep;
	string pattern;
	int lengthPattern;
	int numTimesRepTarget;
	int status;
*/

int map_read_to_ref (string pattern, string line, int lengthRepeat, int numTimes, int targetStartPos, standrep tableRet[5][5][5][5][5], genome * reference_genome, __int64 sizeRef) {
	int lim[5] = {4,4,4,4,4};
	for (int i = 0; i < lengthRepeat; i++) {
		if (line[i] == 'A') lim[i] = 0;
		if (line[i] == 'C') lim[i] = 1;
		if (line[i] == 'G') lim[i] = 2;
		if (line[i] == 'T') lim[i] = 3;
	}
	
	vector<double> significance;
	__int64 tempVar =  tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]].size();
	for (int j = 0; j < tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]].size(); j++) {
		__int64 refStartPos = tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]][j].startPos;
		int refNumRepeat =  tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]][j].numTimesRep;

		int totCounter = 0;
		int matchCounter = 0;
		
		// Pre
		for (int k = 0; k < targetStartPos; k++) {
			if ((refStartPos-1-k) < 0) continue;
			else {
				totCounter++;
				char fromRef = decode((*reference_genome)[2*(refStartPos-1-k)], (*reference_genome)[(2*(refStartPos-1-k))+1]);
				char fromTar = line[targetStartPos-1-k];
				if (fromRef == fromTar) matchCounter++;
			}
		}

		// Post
		int postStartPos = targetStartPos + (lengthRepeat*numTimes);
		int postStartRef = (refStartPos+(lengthRepeat*refNumRepeat));
		int postNum = READ_LENGTH-postStartPos;
		for (int l = 0; l < postNum; l++) {
			if (postStartRef >= (sizeRef*2)) continue;
			else {
				totCounter++;
				char fromRef = decode((*reference_genome)[2*(postStartRef+l)], (*reference_genome)[(2*(postStartRef+l))+1]);
				char fromTar = line[postStartPos+l];
				if (fromRef == fromTar) matchCounter++;
			}
		}

		if (totCounter == 0) significance.push_back(0);
		else significance.push_back((matchCounter/totCounter));
	}
	
	double bestMatchSig = -1;
	int bestMatchIndex = -1;
	for (int m = 0; m < tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]].size(); m++)
		if (significance[m] >= bestMatchSig) bestMatchIndex = m;

	if (bestMatchIndex == -1);// cout << "ERROR: No instance of repeat exists in reference" << endl;
	else cout << "Best Match for read " << line << " starts at " << tableRet[lim[0]][lim[1]][lim[2]][lim[3]][lim[4]][bestMatchIndex].startPos << endl;
	return 0;

}

int find_repeats_in_ref (int num, int i, standrep tableRet[5][5][5][5][5], genome * reference_genome, __int64 sizeRef) {
	
	string fileName = "temp_out_" + to_string(num) + ".txt";
	ifstream myfile (fileName);
	if (myfile.is_open()) {
		string line = "";
		while ( myfile.good()) {
			getline(myfile, line, '\n');
			if (line == "") continue;
			int lengthCount = 0;
			int numRepeat = 0;
			for (int j = 0; j < READ_LENGTH; j++) {
				if (j >= READ_LENGTH-i+1) {
					if (numRepeat >= (MIN_READ_REP-1)) {
						string pattern = "";
						for (int pi = 0; pi < i; pi++) pattern = pattern + line[j-i+pi];
						/*vecSync.lock();
						cout << "Match at line " << line << " and the pattern is ";
						for (int pi = 0; pi < i; pi++) cout << line[j-i+pi];
						cout << " which is repeated " << (numRepeat+1) << " times" << endl;
						vecSync.unlock();*/
						numRepeat = 0;
						map_read_to_ref(pattern, line, i, (numRepeat + 1), (j-((numRepeat + 1)*i)), tableRet, reference_genome, sizeRef);
					}
				}
				else {
					if (j < i);
					else {
						int countCharMatch = 0;
						for (int k = 0; k < i; k++) {
							if (line[j+k] == line[j+k-i])
								countCharMatch++;

							if (countCharMatch == i) {
								if (((i == 4) && (line[j] == line [j+2]) && (line[j+1] == line[j+3]))
									|| ((i == 3) && (line[j] == line [j+1]) && (line[j] == line[j+2]))
									|| ((i == 5) && (line[j] == line [j+1]) && (line[j] == line[j+2]) && (line[j] == line[j+3]) && (line[j] == line[j+4]))) numRepeat = 0;
								else {
									numRepeat++;
									j+=(i-1);
									k=i;
								}
							}

							else {
								if ((numRepeat >= (MIN_READ_REP-1)) && (k == i-1)) {
									string pattern = "";
									for (int pi = 0; pi < i; pi++) pattern = pattern + line[j-i+pi];
									/*vecSync.lock();
									cout << "Match at line " << line << " and the pattern is " << pattern << " which is repeated " << (numRepeat+1) << " times" << endl;
									vecSync.unlock();*/
									numRepeat = 0;
									map_read_to_ref(pattern, line, i, (numRepeat + 1), (j-((numRepeat + 1)*i)), tableRet, reference_genome, sizeRef);
								}
								if ( k== (i-1)) numRepeat = 0;
							}
						}
					}
				}
			}
		}
	}
	return 0;
}

// **Inputs the target reads into a string vector
//   PARAM:  string vector passed by reference (returned with reads)
//   RETVAL: number of reads (i.e. size of vector reads)
int input_target_reads (standrep tableRet[5][5][5][5][5], int num, genome * reference_genome, __int64 sizeRef) {

	cout << "STATUS: Split read buffer thread formed (#" << num << ")" << endl;
	vector<thread *> threadList;

	for (int i = 2; i <= 5 ; ++i)
		threadList.push_back(new thread(find_repeats_in_ref, num, i, tableRet, reference_genome, sizeRef));
	
	for (int j = 0; j < 4; ++j) {
		threadList[j]->join();
		delete threadList[j];
	}

	return 0;
}

int break_up_file() {
	string line;
	ifstream myfile (READ_TARGET);
	ofstream out[NUM_THREADS];
	
	for (int k = 0; k < NUM_THREADS; k++) {
		string num = to_string(k);
		string fileName = "temp_out_" + to_string(k) + ".txt";
		const char * fileNameChar = fileName.c_str ();
		remove(fileNameChar);

		out[k].open(fileName);
		out[k].flags(fstream::out | fstream::app);
	}

	if (myfile.is_open()) {
		while ( myfile.good()) {
			for (int i = 0; i < NUM_THREADS; i++) {
				getline(myfile, line, '\n');
				out[i] << line << '\n';
			}
		}
	}

	for (int j = 0; j < NUM_THREADS; j++) {
		out[j].close();
	}
	myfile.close();
	return 0;
}

int deletefiles() {
	for (int k = 0; k < NUM_THREADS; k++) {
		string num = to_string(k);
		string fileName = "temp_out_" + to_string(k) + ".txt";
		const char * fileNameChar = fileName.c_str ();
		remove(fileNameChar);
	}
	return 0;
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

	// READS PORTION

	break_up_file();

	vector<thread *> threadList;
	for (int i = 0; i < NUM_THREADS; i++) {
		threadList.push_back(new thread(input_target_reads, refRepTable, i, refGenome, sizeRef));
	}

	for (int j = 0; j < NUM_THREADS; j++) {
		threadList[j]->join();
		delete threadList[j];
	}

	// END READS
	cout << "STATUS: All tasks completed successfully ---" << endl << endl;
	outputTable(refRepTable);
	delete(refGenome);
	deletefiles();

}

/*
GAAAGCAGCAGCAGCTCAAATACGGTCATAGTACTCAAACTACTACTACTACTACTACAAAATTATACCATTACGTACGTACGGATCACTAAAGCAAATAACAGATGACTGTAGGTTAGTCAAGATATAAAACAGGCCGAAGACTTGGTTTTGCCCCATAAACAATCAATCCAGTGCTCGCTCGAAACGGAAAGAAAATCAGAGAGGTCGGAAACAGATACAATCGTGGCGGTGGCAGCAGTAGTTAGTTGACGCGGAATGCATCAAGATATATATATATATATATATATATATATATATATATATATATATGGAATAATTTCTTGGCCTCTGCGGCAGCCACTGGCAACCCACATCAAGTTGTTAAGGAAGGTATAACGGTCATCTCCAGTTTCCATGGAGAGGTTAACGCGGTATTTATATGAGTTGGCAGAGGAGGAGGAGGAGTTAATCCCATCCGCAAGTTGAGGGTCACTACTATTGAGCTTATCCAGCACATCTTTCGCGCCTAGCTACGTACGTACGTACGTACGTGTCAACCAC
*/