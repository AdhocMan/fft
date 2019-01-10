all:
	mpic++ -O3 -DNDEBUG -fopenmp test_fft_correctness_1.cpp -I./SDDK -I./ -lstdc++ /Users/antonk/src/LAPACK/scalapack-2.0.2/libscalapack.a -framework Accelerate -framework Accelerate /opt/local/lib/libfftw3.dylib
