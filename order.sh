nvcc -o performance-of-reducer-cuda-2 performance-of-reducer-cuda-2.cu -lcuda

nvcc -o performance-of-reducer-cuda-2-wcopy performance-of-reducer-cuda-2-wcopy.cu -lcuda

g++ -O3 -ftree-vectorize -mavx2 -mfma -fabi-version=0 -I/home/pivarski/.local/include/vectorclass -o performance-of-reducer-sequential performance-of-reducer-sequential.cpp

g++ -O3 -fno-tree-vectorize -mavx2 -mfma -fabi-version=0 -I/home/pivarski/.local/include/vectorclass -o performance-of-reducer-sequential-noautovec performance-of-reducer-sequential.cpp

g++ -O3 -ftree-vectorize -mavx2 -mfma -fabi-version=0 -I/home/pivarski/.local/include/vectorclass -o performance-of-reducer-xsimd performance-of-reducer-xsimd.cpp

for y in sequential sequential-noautovec xsimd cuda-2 cuda-2-wcopy; do echo $y; for x in 0.3 0.5 1.0 2.0 3.0 5.0 10.0 20.0 30.0 50.0 100.0 200.0 300.0 500.0 1000.0; do ./performance-of-reducer-$y DATA/offsets-$x DATA/parents-$x DATA/content-$x; done; done
