// this file will be included from the main project
//

#ifndef __ROCFFT_INTERFACE_HPP__
#define __ROCFFT_INTERFACE_HPP__
namespace rocfft
{

struct rocfft_handler {
    rocfft_plan plan_forward = nullptr;
    rocfft_plan plan_backward = nullptr;
    rocfft_execution_info info = nullptr;
    void* work_buffer = nullptr;
    size_t work_size = 0;
};

void destroy_plan_handle(void* plan);

size_t get_work_size(int ndim, int* dims, int nfft);

void* create_batch_plan(int rank, int* dims, int* embed, int stride, int dist, int nfft,
                        bool auto_alloc);

void set_work_area(void* plan, void* work_area);

void set_stream(void* plan__, stream_id sid__);

void forward_transform(void* plan, std::complex<double>* fft_buffer);

void backward_transform(void* plan, std::complex<double>* fft_buffer);

void initialize();

void finalize();

// void* create_batch_plan(int direction, int rank, int* dims, int dist, int nfft, bool auto_alloc);

// void destroy_plan_handle(void* plan);

// void execute_plan(void* plan, void* buffer);

// size_t get_work_size(void* handler);

// void set_stream(void* plan, stream_id sid);

} // namespace rocfft
#endif
