#include "stdio.h"
#include <ctime>
#include <iostream>

int main(int argc, char** argv) {
  FILE *f_offsets = fopen(argv[1], "r");
  fseek(f_offsets, 0, SEEK_END);
  int num_offsets = ftell(f_offsets) / 4;
  fseek(f_offsets, 0, SEEK_SET);
  int* offsets = (int*)malloc(num_offsets * 4);
  if (!fread(offsets, 4, num_offsets, f_offsets)) return -1;
  
  FILE *f_parents = fopen(argv[2], "r");
  fseek(f_parents, 0, SEEK_END);
  int num_parents = ftell(f_parents) / 4;
  fseek(f_parents, 0, SEEK_SET);
  int* parents = (int*)malloc(num_parents * 4);
  if (!fread(parents, 4, num_parents, f_parents)) return -1;
  
  FILE *f_content = fopen(argv[3], "r");
  fseek(f_content, 0, SEEK_END);
  int num_content = ftell(f_content) / 4;
  fseek(f_content, 0, SEEK_SET);
  float* content = (float*)malloc(num_content * 4);
  if (!fread(content, 4, num_content, f_content)) return -1;

  float* output = (float*)malloc((num_offsets - 1) * 4);

  int numtimes = 10;
  double totaltime = 0.0;
  for (int time = 0;  time < numtimes;  time++) {
    std::clock_t starttime = std::clock();

    for (int i = 0;  i < num_offsets - 1;  i++) {
      output[i] = 0.0;
      for (int j = offsets[i];  j < offsets[i + 1];  j++) {
        output[i] += content[j];
      }
    }

    std::clock_t stoptime = std::clock();    
    totaltime += (stoptime - starttime) / (double)CLOCKS_PER_SEC;
  }

  std::cout << argv[3] << " " << (num_content * numtimes) * 1e-6 / totaltime << " MHz" << std::endl;
  return 0;
}
