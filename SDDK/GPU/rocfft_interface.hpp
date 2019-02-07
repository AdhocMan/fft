// this file will be included from the main project
//

#ifndef __ROCFFT_INTERFACE_HPP__
#define __ROCFFT_INTERFACE_HPP__
#include<complex>

namespace rocfft
{
void destroy_plan_handle(void* plan);

size_t get_work_size(int ndim, int* dims, int nfft);

size_t get_work_size(void* plan);

void* create_batch_plan(int rank, int* dims, int* embed, int stride, int dist, int nfft,
                        bool auto_alloc);

void set_work_area(void* plan, void* work_area);

void set_stream(void* plan__, hipStream_t sid__);

void forward_transform(void* plan, std::complex<double>* fft_buffer);

void backward_transform(void* plan, std::complex<double>* fft_buffer);

void initialize();

void finalize();

} // namespace rocfft
#endif
