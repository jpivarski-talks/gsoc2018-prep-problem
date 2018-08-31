#include "stdio.h"
#include "stdint.h"
#include <ctime>
#include <iostream>
#include <cstring>

typedef int vecint __attribute__ ((vector_size (16)));
typedef float vecfloat __attribute__ ((vector_size (16)));

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

  int numtimes = 1;   // 10;
  double totaltime = 0.0;
  for (int time = 0;  time < numtimes;  time++) {
    memcpy(mutablescan, content, num_content * 4);

    std::clock_t starttime = std::clock();

    vecfloat zero_carry = {0.0, 0.0, 0.0, 0.0};
    vecint step0 = {7, 6, 6, 6};
    vecint step1 = {6, 0, 1, 2};
    vecint step2 = {6, 6, 0, 1};
    vecint tocarry = {6, 6, 6, 3};

    vecint p_zero_carry = {0, 0, 0, 0};

    vecfloat s;
    vecint p;
    for (int i = 0;  i < num_parents;  i += 4) {
      s = *((vecfloat*)&mutablescan[i]);
      p = *((vecint*)&parents[i]);

      s += __builtin_shuffle(s, zero_carry, step0);
      s += __builtin_shuffle(s, zero_carry, step1);
      s += __builtin_shuffle(s, zero_carry, step2);
      zero_carry = __builtin_shuffle(s, zero_carry, tocarry);

      *((vecfloat*)&mutablescan[i]) = s;
    }

    // for (int i = 0;  i < num_offsets - 1;  i++) {
    //   if (offsets[i] == offsets[i + 1]) {
    //     output[i] = 0.0;
    //   }
    //   else {
    //     output[i] = mutablescan[offsets[i + 1] - 1];
    //   }
    // }

    std::clock_t stoptime = std::clock();    
    totaltime += (stoptime - starttime) / (double)CLOCKS_PER_SEC;
  }

  std::cout << (1e3 * totaltime) / numtimes << " ms" << std::endl;
}
