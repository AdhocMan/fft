// this file will be included from the main project
//

#ifndef __ROCFFT_INTERFACE_HPP__
#define __ROCFFT_INTERFACE_HPP__
#include<complex>
#include "acc.hpp"

namespace rocfft
{
void destroy_plan_handle(void* plan);

// NOTE: creates a new plan for work size calculation; if a plan is available call directly with
// pointer to it for better performance
size_t get_work_size(int ndim, int* dims, int nfft);

size_t get_work_size(void* plan);

// embed can be nullptr (stride and dist are then ignored)
void* create_batch_plan(int rank, int* dims, int* embed, int stride, int dist, int nfft,
                        bool auto_alloc);

void set_work_area(void* plan, void* work_area);

void set_stream(void* plan__, stream_id sid__);

void forward_transform(void* plan, std::complex<double>* fft_buffer);

void backward_transform(void* plan, std::complex<double>* fft_buffer);


// function for rocfft library initializeation
// NOTE: Source code in ROCM suggests nothing is actually done (empty functions)
void initialize();

void finalize();

} // namespace rocfft
#endif
