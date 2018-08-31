#include "stdio.h"
#include <ctime>
#include <iostream>
#include <cstring>

int main(int argc, char** argv) {
  FILE *f_offsets = fopen(argv[1], "r");
  fseek(f_offsets, 0, SEEK_END);
  int num_offsets = ftell(f_offsets) / 4;
  fseek(f_offsets, 0, SEEK_SET);
  int* offsets = (int*)malloc(num_offsets * 4);
  fread(offsets, 4, num_offsets, f_offsets);
  
  FILE *f_parents = fopen(argv[2], "r");
  fseek(f_parents, 0, SEEK_END);
  int num_parents = ftell(f_parents) / 4;
  fseek(f_parents, 0, SEEK_SET);
  int* parents = (int*)malloc(num_parents * 4);
  fread(parents, 4, num_parents, f_parents);

  FILE *f_content = fopen(argv[3], "r");
  fseek(f_content, 0, SEEK_END);
  int num_content = ftell(f_content) / 4;
  fseek(f_content, 0, SEEK_SET);
  float* content = (float*)malloc(num_content * 4);
  fread(content, 4, num_content, f_content);

  float* mutablescan = (float*)malloc(num_content * 4);

  float* output = (float*)malloc((num_offsets - 1) * 4);

  int numtimes = 10;
  double totaltime = 0.0;
  for (int time = 0;  time < numtimes;  time++) {
    memcpy(mutablescan, content, num_content * 4);

    std::clock_t starttime = std::clock();

    for (int d = 1;  d < num_parents;  d *= 2) {
      for (int i = d;  i < num_parents;  i++) {
        if (parents[i] == parents[i - d]) {
          mutablescan[i] += mutablescan[i - d];
        }
      }
    }

    for (int i = 0;  i < num_offsets - 1;  i++) {
      if (offsets[i] == offsets[i + 1]) {
        output[i] = 0.0;
      }
      else {
        output[i] = mutablescan[offsets[i + 1] - 1];
      }
    }
    
    std::clock_t stoptime = std::clock();    
    totaltime += (stoptime - starttime) / (double)CLOCKS_PER_SEC;
  }

  std::cout << (1e3 * totaltime) / numtimes << " ms" << std::endl;
}
