// g++ -O3 -ftree-vectorize -march=native -mfma -I/home/pivarski/.local/include/vectorclass -o sequential sequential.cpp

#include "stdio.h"
#include "math.h"
#include <ctime>
#include <iostream>

int main(int argc, char** argv) {
  FILE *f_offsets = fopen(argv[1], "r");
  fseek(f_offsets, 0, SEEK_END);
  int len_offsets = ftell(f_offsets) / 4;
  fseek(f_offsets, 0, SEEK_SET);
  int* offsets = (int*)malloc(int(ceil(len_offsets / 16.0) * 16.0) * 4);
  if (!fread(offsets, 4, len_offsets, f_offsets)) return -1;
  
  FILE *f_parents = fopen(argv[2], "r");
  fseek(f_parents, 0, SEEK_END);
  int len_parents = ftell(f_parents) / 4;
  fseek(f_parents, 0, SEEK_SET);
  int* parents = (int*)malloc(int(ceil(len_parents / 16.0) * 16.0) * 4);
  if (!fread(parents, 4, len_parents, f_parents)) return -1;
  
  FILE *f_content = fopen(argv[3], "r");
  fseek(f_content, 0, SEEK_END);
  int len_content = ftell(f_content) / 4;
  fseek(f_content, 0, SEEK_SET);
  float* content = (float*)malloc(int(ceil(len_content / 16.0) * 16.0) * 4);
  if (!fread(content, 4, len_content, f_content)) return -1;

  float* output = (float*)malloc(int(ceil((len_offsets - 1) / 16.0) * 16.0) * 4);

  std::clock_t starttime = std::clock();

  long numtimes = 10;
  for (long time = 0;  time < numtimes;  time++) {
    for (int i = 0;  i < len_offsets - 1;  i++) {
      output[i] = 0.0;
      for (int j = offsets[i];  j < offsets[i + 1];  j++) {
        output[i] += content[j];
      }
    }
  }

  std::clock_t stoptime = std::clock();    

  std::cout << argv[3] << " " << (len_content * numtimes) * 1e-6 / ((stoptime - starttime) / (double)CLOCKS_PER_SEC) << " MHz" << std::endl;
  return 0;
}
