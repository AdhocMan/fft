// this file will be compiled by hcc

#include <hip/hip_runtime_api.h>
#include <rocfft.h>
#include <stdexcept>
#include "rocfft_interface.hpp"
#include "acc.hpp"

namespace rocfft
{
#define CALL_ROCFFT(func__, args__)                                                     \
    {                                                                                   \
        if ((func__ args__) != rocfft_status_success) {                                 \
            printf("Error in %s at line %i of file %s: ", #func__, __LINE__, __FILE__); \
            exit(-100);                                                                 \
        }                                                                               \
    }

#define CALL_HIP(cmd)                                                                      \
    {                                                                                      \
        hipError_t error = cmd;                                                            \
        if (error != hipSuccess) {                                                         \
            fprintf(stderr, "error: '%s'(%d) at %s:%d\n", hipGetErrorString(error), error, \
                    __FILE__, __LINE__);                                                   \
            exit(EXIT_FAILURE);                                                            \
        }                                                                                  \
    }

struct rocfft_handler {
    rocfft_plan plan_forward = nullptr;
    rocfft_plan plan_backward = nullptr;
    rocfft_execution_info info = nullptr;
    void* work_buffer = nullptr;
    size_t work_size = 0;
};

void initialize() { CALL_ROCFFT(rocfft_setup, ()); }

void finalize() { CALL_ROCFFT(rocfft_cleanup, ()); }

void destroy_plan_handle(void* plan)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);

    // free all device memory
    if (handler->plan_forward != nullptr) CALL_ROCFFT(rocfft_plan_destroy, (handler->plan_forward));
    if (handler->plan_backward != nullptr)
        CALL_ROCFFT(rocfft_plan_destroy, (handler->plan_backward));
    if (handler->info != nullptr) CALL_ROCFFT(rocfft_execution_info_destroy, (handler->info));
    // if (handler->work_buffer != nullptr) CALL_HIP(hipFree(handler->work_buffer));

    // free handler itself
    delete handler;
}

void* create_batch_plan(int rank, int* dims, int* embed, int stride, int dist, int nfft,
                        bool auto_alloc)
{
    // TODO: check how allocation could be implemented
    if (auto_alloc) throw std::runtime_error("Auto allocation for rocfft not implemented!");

    // check input
    for (size_t i = 0; i < rank; i++) {
        if (dims[i] > embed[i])
            throw std::runtime_error("Illegal dims or embed parameters for ROCFFT plan creation!");
    }

    rocfft_plan_description desc = nullptr;

    // ROCFFT appears to expect dimension to be ordered in reverse (see hipFFT implementation)
    size_t lengths[3] = {1, 1, 1};
    for (size_t i = 0; i < rank; i++) lengths[i] = dims[rank - 1 - i];

    if (embed != nullptr) {
        rocfft_plan_description_create(&desc);

        size_t strides[3] = {(size_t)stride, 1, 1};

        size_t nembed_lengths[3] = {1, 1, 1};
        for (size_t i = 0; i < rank; i++) nembed_lengths[i] = embed[rank - 1 - i];

        for (size_t i = 1; i < rank; i++) strides[i] = nembed_lengths[i - 1] * strides[i - 1];

        CALL_ROCFFT(
            rocfft_plan_description_set_data_layout,
            (desc, rocfft_array_type_complex_interleaved, rocfft_array_type_complex_interleaved, 0,
             0, rank, strides, dist, rank, strides, dist));
    }

    rocfft_handler* handler = new rocfft_handler();

    // create plans
    CALL_ROCFFT(rocfft_execution_info_create, (&handler->info));

    CALL_ROCFFT(rocfft_plan_create, (&handler->plan_forward, rocfft_placement_inplace,
                                     rocfft_transform_type_complex_forward, rocfft_precision_double,
                                     rank, lengths, nfft, desc));
    CALL_ROCFFT(rocfft_plan_create, (&handler->plan_backward, rocfft_placement_inplace,
                                     rocfft_transform_type_complex_inverse, rocfft_precision_double,
                                     rank, lengths, nfft, desc));

    // description no longer needed
    CALL_ROCFFT(rocfft_plan_description_destroy, (desc));

    // calculate workbuffer size
    size_t work_size_forward, work_size_backward;
    CALL_ROCFFT(rocfft_plan_get_work_buffer_size, (handler->plan_forward, &work_size_forward));
    CALL_ROCFFT(rocfft_plan_get_work_buffer_size, (handler->plan_backward, &work_size_backward));
    handler->work_size = std::max(work_size_forward, work_size_backward);

    return static_cast<void*>(handler);
}

size_t get_work_size(int ndim, int* dims, int nfft)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(
        create_batch_plan(ndim, dims, nullptr, 1, dims[0], nfft, false));
    const size_t work_size = handler->work_size;
    destroy_plan_handle(handler);
    return work_size;
}

size_t get_work_size(void* plan)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);
    return handler->work_size;
}

void set_work_area(void* plan, void* work_area)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);
    if (handler->work_buffer != nullptr) {
        CALL_HIP(hipFree(handler->work_buffer));
    }
    handler->work_buffer = work_area;
    CALL_ROCFFT(rocfft_execution_info_set_work_buffer,
                (handler->info, work_area, handler->work_size));
}

void set_stream(void* plan__, stream_id sid__)
{
   CALL_ROCFFT(rocfft_execution_info_set_stream,
	       (static_cast<rocfft_handler*>(plan__)->info, acc::stream(sid__)));
}

void forward_transform(void* plan, std::complex<double>* fft_buffer)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);

    void* buffer_array[1];
    buffer_array[0] = (void*)fft_buffer;

    CALL_ROCFFT(rocfft_execute, (handler->plan_forward, buffer_array, buffer_array, handler->info));
}

void backward_transform(void* plan, std::complex<double>* fft_buffer)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);

    void* buffer_array[1];
    buffer_array[0] = (void*)fft_buffer;

    CALL_ROCFFT(rocfft_execute,
                (handler->plan_backward, buffer_array, buffer_array, handler->info));
}
} // namespace rocfft
