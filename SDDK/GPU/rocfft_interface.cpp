// this file will be compiled by hcc

#include <hip/hip_runtime_api.h>
#include <hip/hip_vector_types.h>
#include <rocfft.h>

namespace rocfft {

void initialize()
{
    rocfft_setup();
}

void finalize()
{
    rocfft_cleanup();
}

using rocfft_handler = std::pair<rocfft_plan, rocfft_execution_info>;

void* create_batch_plan(int direction, int rank, int* dims, int dist, int nfft, bool auto_alloc)
{
    auto handler = new rocfft_handler;

    auto fft_direction = (direction == -1) ? rocfft_transform_type_complex_forward : rocfft_transform_type_complex_inverse;

    size_t dimensions = rank;
    size_t length[3];
    for (int i = 0; i < rank; i++) {
        length[i] = dims[i];
    }
    size_t number_of_transforms = nfft;

    rocfft_plan_description description;
    rocfft_plan_description_create(&description);

    size_t distance = dist;

    rocfft_plan_description_set_data_layout(description, rocfft_array_type_complex_interleaved,
                                            rocfft_array_type_complex_interleaved, nullptr, nullptr,
                                            dimensions, nullptr, distance, dimensions, nullptr, distance);

    rocfft_plan_create(&handler->first, rocfft_placement_inplace, fft_direction, rocfft_precision_double,
                       dimensions, length, number_of_transforms, description);

    rocfft_plan_description_destroy(description);

    rocfft_execution_info_create(&handler->second);

    return handler;
}

void destroy_plan(void* handler)
{
    rocfft_plan_destroy(static_cast<rocfft_handler*>(handler)->first);
    rocfft_execution_info_destroy(static_cast<rocfft_handler*>(handler)->second);
    delete static_cast<rocfft_handler*>(handler);
}

void execute_plan(void* handler, void* buffer)
{
    rocfft_execute(static_cast<rocfft_handler*>(handler)->first, &buffer, NULL,
                   static_cast<rocfft_handler*>(handler)->second);
}

size_t get_work_size(void* handler)
{
    size_t work_size;

    rocfft_plan_get_work_buffer_size(static_cast<rocfft_handler*>(handler)->first, &work_size);

    return work_size;
}

}
