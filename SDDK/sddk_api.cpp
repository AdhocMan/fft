// Copyright (c) 2013-2018 Anton Kozhevnikov, Thomas Schulthess
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that
// the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this list of conditions and the
//    following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions
//    and the following disclaimer in the documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
// WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
// ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

/** \file sddk_api.cpp
 *
 *  \brief Fortran interface for the SDDK library.
 */

#include "sddk.hpp"
#include "../utils/any_ptr.hpp"

using namespace sddk;

using ftn_int            = int32_t;
using ftn_double         = double;
using ftn_logical        = bool;
using ftn_double_complex = std::complex<double>;

extern "C" {

void sddk_init()
{
    /* something useful can be done here */
}

void sddk_delete_object(void** handler__)
{
    delete static_cast<utils::any_ptr*>(*handler__);
}

///// Create FFT grid.
//void sddk_create_fft_grid(ftn_int* dims__,
//                          ftn_int* new_object_id__) 
//{
//    int id = get_next_free_object_id();
//    sddk_objects[id] = new FFT3D_grid({dims__[0], dims__[1], dims__[2]});
//    sddk_objects_class_name[id] = "FFT3D_grid";
//    *new_object_id__ = id;
//}

/// Create list of G-vectors.
void sddk_create_gvec(double const* b1__,
                      double const* b2__,
                      double const* b3__,
                      double const* gmax__,
                      bool   const* reduce_gvec__,
                      int    const* fcomm__,
                      void**        handler__)
{
    auto& comm = Communicator::map_fcomm(*fcomm__);

    matrix3d<double> lat_vec;
    for (int x: {0, 1, 2}) {
        lat_vec(x, 0) = b1__[x];
        lat_vec(x, 1) = b2__[x];
        lat_vec(x, 2) = b3__[x];
    }
    *handler__ = new utils::any_ptr(new Gvec(lat_vec, *gmax__, comm, *reduce_gvec__));
}

/// Create list of G+k-vectors.
void sddk_create_gkvec(double const* vk__,
                       double const* b1__,
                       double const* b2__,
                       double const* b3__,
                       double const* gmax__,
                       bool   const* reduce_gvec__,
                       int    const* fcomm__,
                       void**        handler__)
{
    auto& comm = Communicator::map_fcomm(*fcomm__);

    matrix3d<double> lat_vec;
    for (int x: {0, 1, 2}) {
        lat_vec(x, 0) = b1__[x];
        lat_vec(x, 1) = b2__[x];
        lat_vec(x, 2) = b3__[x];
    }
    *handler__ = new utils::any_ptr(new Gvec({vk__[0], vk__[1], vk__[2]}, lat_vec, *gmax__, comm, *reduce_gvec__));
}

void sddk_create_gvec_partition(void* const* gvec_handler__,
                                int   const* fft_comm__,
                                int   const* comm_ortho_fft__,
                                void**       handler__)
{
    auto& gv = static_cast<utils::any_ptr*>(*gvec_handler__)->get<Gvec>();
    auto& fft_comm = Communicator::map_fcomm(*fft_comm__);
    auto& comm_ortho_fft = Communicator::map_fcomm(*comm_ortho_fft__);

    *handler__ = new utils::any_ptr(new Gvec_partition(gv, fft_comm, comm_ortho_fft));
}

/// Create FFT driver.
void sddk_create_fft(int const* initial_dims__,
                     int const* fcomm__,
                     void**     handler__)
{
    auto& comm = Communicator::map_fcomm(*fcomm__);
    *handler__ = new utils::any_ptr(new FFT3D({initial_dims__[0], initial_dims__[1], initial_dims__[2]}, comm, device_t::CPU));
}

/// Create wave functions.
void sddk_create_wave_functions(ftn_int* gkvec_id__,
                                ftn_int* num_wf__,
                                ftn_int* new_object_id__)
{
    //int id = get_next_free_object_id();
    //auto& gkvec = *reinterpret_cast<Gvec*>(sddk_objects[*gkvec_id__]);

    TERMINATE("pass number of spins");
    //sddk_objects[id] = new Wave_functions(gkvec, *num_wf__, 1);
    //sddk_objects_class_name[id] = "Wave_functions";
    //*new_object_id__ = id;
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[id]);
    //wf.pw_coeffs().prime(0, 0) = double_complex(12.13, 14.15);
    //wf.pw_coeffs().prime(0, 1) = double_complex(1, 2);
}

void sddk_remap_wave_functions_forward(ftn_int* wf_id__, ftn_int* n__, ftn_int* idx0__)
{
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[*wf_id__]);
    STOP();
    //wf.pw_coeffs().remap_forward(CPU, wf.gkvec().partition().gvec_fft_slab(), *n__, *idx0__ - 1);
}

void sddk_remap_wave_functions_backward(ftn_int* wf_id__, ftn_int* n__, ftn_int* idx0__)
{
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[*wf_id__]);
    STOP();
    //wf.pw_coeffs().remap_backward(CPU, wf.gkvec().partition().gvec_fft_slab(), *n__, *idx0__ - 1);
}

void sddk_get_num_wave_functions(ftn_int* wf_id__, ftn_int* num_wf__)
{
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[*wf_id__]);
    //*num_wf__ = wf.num_wf();
}

void sddk_get_num_wave_functions_local(ftn_int* wf_id__, ftn_int* num_wf__)
{
    STOP();
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[*wf_id__]);
    //*num_wf__ = wf.pw_coeffs().spl_num_col().local_size();
}

void sddk_get_wave_functions_prime_ld(ftn_int* wf_id__, ftn_int* ld__)
{
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[*wf_id__]);
    STOP();
    //*ld__ = wf.pw_coeffs().prime().ld();
}

void sddk_get_wave_functions_extra_ld(ftn_int* wf_id__, ftn_int* ld__)
{
    STOP();
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[*wf_id__]);
    //*ld__ = wf.pw_coeffs().extra().ld();
}

void sddk_get_wave_functions_prime_ptr(ftn_int* wf_id__,
                                       ftn_double_complex** ptr__)
{
    STOP();
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[*wf_id__]);
    //*ptr__ = wf.pw_coeffs().prime().at<CPU>();
}

void sddk_get_wave_functions_extra_ptr(ftn_int* wf_id__,
                                       ftn_double_complex** ptr__)
{
    STOP();
    //auto& wf = *reinterpret_cast<Wave_functions*>(sddk_objects[*wf_id__]);
    //*ptr__ = wf.pw_coeffs().extra().at<CPU>();
}

/// Get total number of G-vectors.
void sddk_get_num_gvec(ftn_int* gvec_id__, ftn_int* num_gvec__)
{
    //*num_gvec__ = reinterpret_cast<Gvec*>(sddk_objects[*gvec_id__])->num_gvec();
}

/// Get local number of G-vectors in the fine-graind distribution.
void sddk_get_gvec_count(ftn_int* gvec_id__,
                         ftn_int* rank__,
                         ftn_int* gvec_count__)
{
    //*gvec_count__ = reinterpret_cast<Gvec*>(sddk_objects[*gvec_id__])->gvec_count(*rank__);
}

/// Get index offset of G-vectors in the fine-graind distribution.
void sddk_get_gvec_offset(ftn_int* gvec_id__,
                          ftn_int* rank__,
                          ftn_int* gvec_offset__)
{
    //*gvec_offset__ = reinterpret_cast<Gvec*>(sddk_objects[*gvec_id__])->gvec_offset(*rank__);
}

///// Get local number of G-vectors for the FFT.
//void sddk_get_gvec_count_fft(ftn_int* gvec_id__,
//                             ftn_int* gvec_count__)
//{
//    *gvec_count__ = reinterpret_cast<Gvec*>(sddk_objects[*gvec_id__])->partition().gvec_count_fft();
//}
//
///// Get index offset of G-vectors for the FFT.
//void sddk_get_gvec_offset_fft(ftn_int* gvec_id__,
//                              ftn_int* gvec_offset__)
//{
//    *gvec_offset__ = reinterpret_cast<Gvec*>(sddk_objects[*gvec_id__])->partition().gvec_offset_fft();
//}

void sddk_fft(ftn_int*            fft_id__,
              ftn_int*            direction__,
              ftn_double_complex* data__)
{
    //switch (*direction__) {
    //    case 1: {
    //        reinterpret_cast<FFT3D*>(sddk_objects[*fft_id__])->transform<1>(data__);
    //        break;
    //    }
    //    case -1: {
    //        reinterpret_cast<FFT3D*>(sddk_objects[*fft_id__])->transform<-1>(data__);
    //        break;
    //    }
    //    default: {
    //        TERMINATE("wrong FFT direction");
    //    }
    //}
}

void sddk_fft_prepare(ftn_int* fft_id__,
                      ftn_int* gvec_id__)
{
    //auto gv = reinterpret_cast<Gvec*>(sddk_objects[*gvec_id__]);
    //reinterpret_cast<FFT3D*>(sddk_objects[*fft_id__])->prepare(gv->partition());
}

void sddk_fft_dismiss(ftn_int* fft_id__)
{
    //reinterpret_cast<FFT3D*>(sddk_objects[*fft_id__])->dismiss();
}

void sddk_print_timers()
{
    utils::timer::print();
}


}
