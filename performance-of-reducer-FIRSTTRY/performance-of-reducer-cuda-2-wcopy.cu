#include "stdio.h"
#include <ctime>
#include <iostream>

__global__ void reduce_a(int lenparents, int* parents, float* mutablescan) {
  unsigned int i = threadIdx.x + blockIdx.x*blockDim.x;

  for (int d = 1;  d < 1024;  d *= 2) {
    if (i < lenparents  &&  i >= d  &&  parents[i] == parents[i - d]) {
      mutablescan[i] = mutablescan[i] + mutablescan[i - d];
    }
    __syncthreads();
  }
}

__global__ void reduce_b(int lenparents, int* parents, float* mutablescan) {
  unsigned int i = (threadIdx.x + blockIdx.x*blockDim.x + 1) * 1024;
  
  int extra = 0;
  while (i + extra < lenparents  &&  extra < 1024  &&  parents[i + extra] == parents[i - 1]) {
    mutablescan[i + extra] += mutablescan[i - 1];
    extra++;
  }
}

__global__ void reduce_c(int lenstarts, int* offsets, float* scan, float* output) {
  unsigned int i = threadIdx.x + blockIdx.x*blockDim.x;

  if (i < lenstarts) {
    if (offsets[i] == offsets[i + 1]) {
      output[i] = 0.0;
    }
    else {
      output[i] = scan[offsets[i + 1] - 1];
    }
  }
}

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
  
  int* gpu_offsets;
  int* gpu_parents;
  float* gpu_content;
  float* gpu_output;
  
  cudaMalloc((void**)&gpu_offsets, num_offsets * 4);
  cudaMalloc((void**)&gpu_parents, num_parents * 4);
  cudaMalloc((void**)&gpu_content, num_content * 4);
  cudaMalloc((void**)&gpu_output, (num_offsets - 1) * 4);

  float* output = (float*)malloc((num_offsets - 1) * 4);

  int numtimes = 10;
  double totaltime = 0.0;
  for (int time = 0;  time < numtimes;  time++) {
    std::clock_t starttime = std::clock();

    cudaMemcpy(gpu_offsets, offsets, num_offsets * 4, cudaMemcpyHostToDevice);
    cudaMemcpy(gpu_parents, parents, num_parents * 4, cudaMemcpyHostToDevice);
    cudaMemcpy(gpu_content, content, num_content * 4, cudaMemcpyHostToDevice);
    cudaDeviceSynchronize();

    int threadsperblock = 1024;
    int blocksize = num_parents / threadsperblock + 1;

    reduce_a<<<blocksize, threadsperblock>>>(num_parents, gpu_parents, gpu_content);

    blocksize = (num_parents / 1024) / threadsperblock + 1;
    reduce_b<<<blocksize, threadsperblock>>>(num_parents, gpu_parents, gpu_content);

    blocksize = (num_offsets - 1) / threadsperblock + 1;
    reduce_c<<<blocksize, threadsperblock>>>(num_offsets - 1, gpu_offsets, gpu_content, gpu_output);

    cudaMemcpy(output, gpu_output, (num_offsets - 1) * 4, cudaMemcpyDeviceToHost);
    std::clock_t stoptime = std::clock();

    totaltime += (stoptime - starttime) / (double)CLOCKS_PER_SEC;
  }

  std::cout << argv[3] << " " << (num_content * numtimes) * 1e-6 / totaltime << " MHz" << std::endl;
}
