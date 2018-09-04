#include "stdio.h"
#include "math.h"
#include "string.h"
#include <ctime>
#include <iostream>

#define MAX_VECTOR_SIZE 512
#include "vectorclass.h"

int main(int argc, char** argv) {
  FILE *f_offsets = fopen(argv[1], "r");
  fseek(f_offsets, 0, SEEK_END);
  int num_offsets = ftell(f_offsets) / 4;
  fseek(f_offsets, 0, SEEK_SET);
  int* offsets = (int*)malloc(int(ceil(num_offsets / 16.0) * 16.0) * 4);
  if (!fread(offsets, 4, num_offsets, f_offsets)) return -1;
  // for (int i = num_offsets;  i < int(ceil(num_offsets / 16.0) * 16.0);  i++)
  //   offsets[i] = -1;
  
  FILE *f_parents = fopen(argv[2], "r");
  fseek(f_parents, 0, SEEK_END);
  int num_parents = ftell(f_parents) / 4;
  fseek(f_parents, 0, SEEK_SET);
  int* parents = (int*)malloc(int(ceil(num_parents / 16.0) * 16.0 + 1.0) * 4);
  if (!fread(parents, 4, num_parents, f_parents)) return -1;
  // for (int i = 0;  i < num_parents + 1;  i++) {
  //   parents[i] = -1;
  // }

  FILE *f_content = fopen(argv[3], "r");
  fseek(f_content, 0, SEEK_END);
  int num_content = ftell(f_content) / 4;
  fseek(f_content, 0, SEEK_SET);
  float* content = (float*)malloc(int(ceil(num_content / 16.0) * 16.0) * 4);
  if (!fread(content, 4, num_content, f_content)) return -1;

  float* output = (float*)malloc(int(ceil((num_offsets - 1) / 16.0) * 16.0) * 4);

  long numtimes = 30;
  double totaltime = 0.0;
  for (long time = 0;  time < numtimes;  time++) {
    std::clock_t starttime = std::clock();

    memset(output, 0, sizeof(float) * (num_offsets - 1));

    Vec16f s;
    Vec16f scarry = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    Vec16i p;
    Vec16i pcarry = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    Vec16i p2;
    Vec16i p2none = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    // Vec16f tmp;

    for (int i = 0;  i < num_parents;  i += 16) {
        s.load(&content[i]);
        p.load(&parents[i]);
        p2.load(&parents[i + 1]);

        s = if_add(p == pcarry, s, scarry);

        s = if_add(p == permute16i<-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14>(p),
                   s,   permute16f<-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14>(s));

        s = if_add(p == permute16i<-1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13>(p),
                   s,   permute16f<-1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13>(s));

        s = if_add(p == permute16i<-1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11>(p),
                   s,   permute16f<-1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11>(s));

        s = if_add(p == permute16i<-1, -1, -1, -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7>(p),
                   s,   permute16f<-1, -1, -1, -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7>(s));

        scarry = blend16f<15, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31>(s, scarry);
        pcarry = blend16i<15, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31>(p, pcarry);

        scatter(select(p == p2, p2none, p), num_offsets - 1, s, output);

        // tmp += s;
    }

    // std::cout << tmp[0] << std::endl;

    std::clock_t stoptime = std::clock();    
    totaltime += (stoptime - starttime) / (double)CLOCKS_PER_SEC;
  }

  std::cout << argv[3] << " " << (num_content * numtimes) * 1e-6 / totaltime << " MHz; offsets " << num_offsets << std::endl;
  return 0;
}

/*
void vectorized16_max_impl(int32_t len_offsets, int32_t len_parents, int32_t* parents, float* content, float* output) {
    for (int i = 0;  i < len_offsets - 1;  i++) {
        output[i] = -INFINITY;
    }
    
    Vec16f s;
    Vec16f scarry = {-INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY,
                     -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY};
    Vec16f szero  = {-INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY,
                     -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY, -INFINITY};
    Vec16f sblend;
    Vec16i p;
    Vec16i pcarry = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    Vec16i p2;
    Vec16i p2none = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

    for (int i = 0;  i < len_parents;  i += 16) {
        s.load(&content[i]);
        p.load(&parents[i]);
        p2.load(&parents[i + 1]);

        s = select((p == pcarry) & (scarry > s), scarry, s);

        sblend           = blend16f<16,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14>(s, szero);
        s = select((p == permute16i<-1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14>(p)) & (sblend > s), sblend, s);

        sblend           = blend16f<16, 16,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13>(s, szero);
        s = select((p == permute16i<-1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13>(p)) & (sblend > s), sblend, s);

        sblend           = blend16f<16, 16, 16, 16,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11>(s, szero);
        s = select((p == permute16i<-1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11>(p)) & (sblend > s), sblend, s);

        sblend           = blend16f<16, 16, 16, 16, 16, 16, 16, 16,  0,  1,  2,  3,  4,  5,  6,  7>(s, szero);
        s = select((p == permute16i<-1, -1, -1, -1, -1, -1, -1, -1,  0,  1,  2,  3,  4,  5,  6,  7>(p)) & (sblend > s), sblend, s);

        scarry = blend16f<15, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31>(s, scarry);
        pcarry = blend16i<15, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31>(p, pcarry);

        scatter(select(p == p2, p2none, p), len_offsets, s, output);
    }
}
*/
