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

}
