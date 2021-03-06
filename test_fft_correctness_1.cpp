#include "sirius.h"

/* test FFT: transform single harmonic and compare with plane wave exp(iGr) */

using namespace sirius;

template <memory_t mem__>
int test_fft(cmd_args& args, device_t pu__)
{
    double cutoff = args.value<double>("cutoff", 10);

    matrix3d<double> M = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};

    FFT3D fft(find_translations(cutoff, M), Communicator::world(), pu__);

    std::cout << "FFT grid: " << fft.size(0) << " " << fft.size(1) << " " << fft.size(2) << "\n";

    Gvec gvec(M, cutoff, Communicator::world(), false);
    Gvec_partition gvecp(gvec, Communicator::world(), Communicator::self());

    fft.prepare(gvecp);

    mdarray<double_complex, 1> f(gvec.num_gvec());
    if (pu__ == GPU) {
        f.allocate(memory_t::device);
    }
    mdarray<double_complex, 1> ftmp(gvecp.gvec_count_fft());
    if (pu__ == GPU) {
        ftmp.allocate(memory_t::device);
    }

    int result{0};

    for (int ig = 0; ig < gvec.num_gvec(); ig++) {
        auto v = gvec.gvec(ig);
        f.zero();
        f[ig] = 1.0;
        /* load local set of PW coefficients */
        for (int igloc = 0; igloc < gvecp.gvec_count_fft(); igloc++) {
            ftmp[igloc] = f[gvecp.idx_gvec(igloc)];
        }
        switch (pu__) {
            case CPU: {
                fft.transform<1>(&ftmp[0]);
                break;
            }
            case GPU: {
                ftmp.copy_to(memory_t::device);
                fft.transform<1, mem__>(ftmp.at(mem__));
                fft.buffer().copy_to(memory_t::host);
                break;
            }
        }

        double const twopi = 6.28318530717958647692528676656;

        double diff = 0;
        /* loop over 3D array (real space) */
        for (int j0 = 0; j0 < fft.size(0); j0++) {
            for (int j1 = 0; j1 < fft.size(1); j1++) {
                for (int j2 = 0; j2 < fft.local_size_z(); j2++) {
                    /* get real space fractional coordinate */
                    auto rl = vector3d<double>(double(j0) / fft.size(0), 
                                               double(j1) / fft.size(1), 
                                               double(fft.offset_z() + j2) / fft.size(2));
                    int idx = fft.index_by_coord(j0, j1, j2);

                    /* compare value with the exponent */
                    diff += std::pow(std::abs(fft.buffer(idx) - std::exp(double_complex(0.0, twopi * dot(rl, v)))), 2);
                }
            }
        }
        Communicator::world().allreduce(&diff, 1);
        diff = std::sqrt(diff / fft.size());
        if (diff > 1e-10) {
            result++;
        }
    }

    fft.dismiss();

    return result;
}

int run_test(cmd_args& args)
{
    int result = test_fft<memory_t::host>(args, device_t::CPU);
    if (Communicator::world().rank() == 0) {
        printf("running on CPU: number of errors: %i\n", result);
    }
#ifdef __GPU
    int result1 = test_fft<memory_t::host>(args, device_t::GPU);
    if (Communicator::world().rank() == 0) {
        printf("running on GPU, using host memory pointer: number of errors: %i\n", result1);
    }
    result += result1;
    result1 = test_fft<memory_t::device>(args, device_t::GPU);
    if (Communicator::world().rank() == 0) {
        printf("running on GPU, using device memory pointer: number of errors: %i\n", result1);
    }
    result += result1;
#endif
    return result;
}

int main(int argn, char **argv)
{
    cmd_args args;
    args.register_key("--cutoff=", "{double} cutoff radius in G-space");

    args.parse_args(argn, argv);
    if (args.exist("help")) {
        printf("Usage: %s [options]\n", argv[0]);
        args.print_help();
        return 0;
    }

    sirius::initialize(true);
    int result = run_test(args);
    if (Communicator::world().rank() == 0) {
        if (result) {
            printf("\x1b[31m" "Failed" "\x1b[0m" "\n");
        } else {
            printf("\x1b[32m" "OK" "\x1b[0m" "\n");
        }
    }
    sirius::finalize();

    return result;
}
