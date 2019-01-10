#ifndef __FFT_DEVICE_HPP__
#define __FFT_DEVICE_HPP__

namespace acc {

namespace fft {

#if defined(__CUDA)
using fft_plan_device_handler_t = cufftHandle;
#elif defined(__ROCM)
using fft_plan_device_handler_t = rocfft_plan;
#else
using fft_plan_device_handler_t = void*;
#endif

void initialize();
void finalize();

}

}

#endif
