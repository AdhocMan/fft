// this file will be compiled by hcc

#include <hip/hip_runtime_api.h>
// #include <hip/hip_vector_types.h>
// #include <hipfft.h>
#include <rocfft.h>
#include <stdexcept>
// #include "rocfft_interface.hpp"

namespace rocfft {
// #define CALL_HIPFFT(func__, args__)                                                  \
// {                                                                                   \
//     if (( func__ args__) != HIPFFT_SUCCESS) {                                \
//         printf("Error in %s at line %i of file %s: ", #func__, __LINE__, __FILE__); \
//         exit(-100);                                                                 \
//     }                                                                               \
// }
#define CALL_ROCFFT(func__, args__)                                                     \
    {                                                                                   \
        if ((func__ args__) != rocfft_status_success) {                                 \
            printf("Error in %s at line %i of file %s: ", #func__, __LINE__, __FILE__); \
            exit(-100);                                                                 \
        }                                                                               \
    }


void initialize() { CALL_ROCFFT(rocfft_setup, ()); }

void finalize() { CALL_ROCFFT(rocfft_cleanup, ()); }

void destroy_plan_handle(void* plan)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);

    if (handler->plan_forward != nullptr) CALL_ROCFFT(rocfft_plan_destroy, (handler->plan_forward));
    if (handler->plan_backward != nullptr)
        CALL_ROCFFT(rocfft_plan_destroy, (handler->plan_backward));
    if (handler->info != nullptr) CALL_ROCFFT(rocfft_execution_info_destroy, (handler->info));
    hipFree(handler->work_buffer);
    delete handler;
}

size_t get_work_size(int ndim, int* dims, int nfft)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(
        create_batch_plan(ndim, dims, nullptr, 1, dims[0], nfft, false));
    const size_t work_size = handler->work_size;
    destroy_plan_handle(handler);
    return work_size;
}

void* create_batch_plan(int rank, int* dims, int* embed, int stride, int dist, int nfft,
                        bool auto_alloc)
{
    if (auto_alloc) throw std::runtime_error("Auto allocation for rocfft not implemented!");

    rocfft_plan_description desc = nullptr;

    // TODO: Check if swap of dims is correct (HIPFFT does it when translating to ROCFFT)
    size_t lengths[3];
    for (size_t i = 0; i < rank; i++) lengths[i] = dims[rank - 1 - i];

    if (embed != nullptr) {
        rocfft_plan_description_create(&desc);

        size_t strides[3] = {stride, 1, 1};

        size_t nembed_lengths[3];
        for (size_t i = 0; i < rank; i++) nembed_lengths[i] = embed[rank - 1 - i];

        for (size_t i = 1; i < rank; i++) strides[i] = nembed_lengths[i - 1] * strides[i - 1];

        CALL_ROCFFT(
            rocfft_plan_description_set_data_layout,
            (desc, rocfft_array_type_complex_interleaved, rocfft_array_type_complex_interleaved, 0,
             0, rank, strides, dist, rank, strides, dist));
    }

    rocfft_handler* handler = new rocfft_handler();

    CALL_ROCFFT(rocfft_plan_create,
                (&handler->plan_forward, rocfft_placement_inplace, rocfft_placement_inplace,
                 rocfft_precision_double, rank, lengths, nfft, desc));
    CALL_ROCFFT(rocfft_plan_create,
                (&handler->plan_backward, rocfft_placement_inplace, rocfft_placement_inplace,
                 rocfft_precision_double, rank, lengths, nfft, desc));

    CALL_ROCFFT(rocfft_plan_description_destroy, (desc));

    size_t work_size_forward, work_size_backward;
    CALL_ROCFFT(rocfft_plan_get_work_buffer_size, (handler->plan_forward, &work_size_forward));
    CALL_ROCFFT(rocfft_plan_get_work_buffer_size, (handler->plan_backward, &work_size_backward));
    handler->work_size = std::max(work_size_forward, work_size_backward);
    return static_cast<void*>(handler);
}

void set_work_area(void* plan, void* work_area)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);
    if (handler->work_buffer != nullptr) {
        hipFree(handler->work_buffer);
    }
    handler->work_buffer = work_area;
    CALL_ROCFFT(rocfft_execution_info_set_work_buffer,
                (handler->info, work_area, handler->work_size));
}

void set_stream(void* plan__, stream_id sid__)
{
    CALL_ROCFFT(rocfft_execution_info_set_stream,
                (static_cast<rocfft_handler*>(plan__)->info, sid__));
}

void forward_transform(void* plan, std::complex<double>* fft_buffer)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);

    void* buffer_array[1];
    buffer_array[0] = (void*)fft_buffer;

    CALL_ROCFFT(rocfft_execute(handler->plan_forward, buffer_array, buffer_array, hanlder->info));
}

void backward_transform(void* plan, std::complex<double>* fft_buffer)
{
    rocfft_handler* handler = static_cast<rocfft_handler*>(plan);

    void* buffer_array[1];
    buffer_array[0] = (void*)fft_buffer;

    CALL_ROCFFT(rocfft_execute(handler->plan_backward, buffer_array, buffer_array, hanlder->info));
}

// struct rocfft_handler {
//     hipfftHandle plan;
//     size_t work_size;
// };

// static void* create_batch_plan_internal(int rank, int* dims, int* embed, int stride, int dist, int nfft, bool auto_alloc, size_t* work_size) {
//     if(auto_alloc) {
//         throw std::runtime_error("Auto allocation not implentend (Not supported by HipFFT yet)!");
//     }
//     rocfft_handler* handler = new rocfft_handler();
//     CALL_HIPFFT(hipfftMakePlanMany, (&handler->plan, rank, dims, embed, stride, dist, embed, stride, dist, HIPFFT_Z2Z, nfft, &handler->work_size));
//     return static_cast<void*>(handler);
// }

// void destroy_plan_handle(void* handle)
// {
//     CALL_HIPFFT(hipfftDestroy, (static_cast<rocfft_handler*>(handle)->plan));
//     delete static_cast<rocfft_handler*>(handle);
// }

// size_t get_work_size(int ndim, int* dims, int nfft)
// {
//     int fft_size = 1;
//     for (int i = 0; i < ndim; i++) {
//         fft_size *= dims[i];
//     }
//     size_t work_size;
//     //TODO: check parameters
//     void* plan = create_batch_plan_internal(ndim, dims, nullptr, 1, fft_size, nfft, false, &work_size);
//     CALL_HIPFFT(hipfftDestroy, (plan));
//     return work_size;
//     //TODO
// }

// size_t get_work_size(void* handle)
// {
//     return static_cast<rocfft_handler>(handle)->work_size;
// }

// void* create_batch_plan(int rank, int* dims, int* embed, int stride, int dist, int nfft, bool auto_alloc)
// {
//     size_t work_size;
//     hipfftHandle* handle =create_batch_plan_internal(rank, dims, embed, stride, dist, nfft, auto_alloc, work_size);

//     return static_cast<void*>(handle);
// }

// void set_work_area(void* plan, void* work_area)
// {
//     // SetWorkArea not yet implemented by HipFFT, use ROCFFT instead
//     ROC_FFT_CHECK_INVALID_VALUE(rocfft_execution_info_set_work_buffer, (plan->info, plan->workBuffer, workBufferSize));
//     //TODO
// }

// void set_stream(void* plan__, stream_id sid__)
// {
//     //TODO
// }

// void forward_transform(void* plan, std::complex<double>* fft_buffer)
// {
//     //TODO
// }

// void backward_transform(void* plan, std::complex<double>* fft_buffer)
// {
//     //TODO
// }

// void initialize()
// {
//     rocfft_setup();
// }

// void finalize()
// {
//     rocfft_cleanup();
// }

// using rocfft_handler = std::pair<rocfft_plan, rocfft_execution_info>;

// void* create_batch_plan(int direction, int rank, int* dims, int dist, int nfft, bool auto_alloc)
// {
//     auto handler = new rocfft_handler;

//     auto fft_direction = (direction == -1) ? rocfft_transform_type_complex_forward : rocfft_transform_type_complex_inverse;

//     size_t dimensions = rank;
//     size_t length[3];
//     for (int i = 0; i < rank; i++) {
//         length[i] = dims[i];
//     }
//     size_t number_of_transforms = nfft;

//     rocfft_plan_description description;
//     rocfft_plan_description_create(&description);

//     size_t distance = dist;

//     rocfft_plan_description_set_data_layout(description, rocfft_array_type_complex_interleaved,
//                                             rocfft_array_type_complex_interleaved, nullptr, nullptr,
//                                             dimensions, nullptr, distance, dimensions, nullptr, distance);

//     rocfft_plan_create(&handler->first, rocfft_placement_inplace, fft_direction, rocfft_precision_double,
//                        dimensions, length, number_of_transforms, description);

//     rocfft_plan_description_destroy(description);

//     rocfft_execution_info_create(&handler->second);

//     return handler;
// }

// void destroy_plan_handle(void* handler)
// {
//     rocfft_plan_destroy(static_cast<rocfft_handler*>(handler)->first);
//     rocfft_execution_info_destroy(static_cast<rocfft_handler*>(handler)->second);
//     delete static_cast<rocfft_handler*>(handler);
// }

// void execute_plan(void* handler, void* buffer)
// {
//     rocfft_execute(static_cast<rocfft_handler*>(handler)->first, &buffer, NULL,
//                    static_cast<rocfft_handler*>(handler)->second);
// }

// size_t get_work_size(void* handler)
// {
//     size_t work_size;

//     rocfft_plan_get_work_buffer_size(static_cast<rocfft_handler*>(handler)->first, &work_size);

//     return work_size;
// }

// void set_stream(void* plan, stream_id sid) {
// }

// TODO: set work size
}
