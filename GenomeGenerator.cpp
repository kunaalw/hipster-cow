// C++ libraries
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <stdio.h>
#include <thread>

#define GENOME_SIZE 30000000
#define OUT_FILE "gen_wip.txt"


using namespace std;

int main()
{
   ofstream outdata;
   int j;
  outdata.open(OUT_FILE);
   if( !outdata ) {
      cerr << "Error: file could not be opened" << endl;
      exit(1);
   }
  for (j=0; j<GENOME_SIZE; ++j) {
	  int i = rand()%4;
	  char c = 'X';
	  if (i == 0) c = 'A';
	  else if (i == 1) c = 'C';
	  else if (i == 2) c = 'G';
	  else if (i == 3) c = 'T';
	  outdata << c;
  }
   outdata.close();

   return 0;
}
