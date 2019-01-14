// Copyright (c) 2013-2016 Anton Kozhevnikov, Thomas Schulthess
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

/** \file linalg.hpp
 *
 *  \brief Linear algebra interface.
 */

#ifndef __LINALG_HPP__
#define __LINALG_HPP__

#include <stdint.h>
#if defined(__GPU) && defined(__CUDA)
#include "GPU/cublas.hpp"
#endif
#ifdef __MAGMA
#include "GPU/magma.hpp"
#endif
#include "blas_lapack.h"
#include "memory.hpp"
#include "dmatrix.hpp"
#include "GPU/acc.hpp"

namespace sddk {

/// Linear algebra interface class.
template <device_t pu>
class linalg;

template<>
class linalg<CPU>: public linalg_base
{
    public:

        /// General matrix times a vector.
        /** Perform one of the matrix-vector operations \n
         *  y = alpha * A * x + beta * y (trans = 0) \n
         *  y = alpha * A^{T} * x + beta * y (trans = 1) \n
         *  y = alpha * A^{+} * x + beta * y (trans = 2)
         */
        template<typename T>
        static void gemv(int trans, ftn_int m, ftn_int n, T alpha, T const* A, ftn_int lda, T const* x, ftn_int incx,
                         T beta, T* y, ftn_int incy);

        template<typename T>
        static void ger(ftn_int m, ftn_int n, T alpha, T* x, ftn_int incx, T* y, ftn_int incy, T* A, ftn_int lda);

        /// Hermitian matrix times a general matrix or vice versa.
        /** Perform one of the matrix-matrix operations \n
         *  C = alpha * A * B + beta * C (side = 0) \n
         *  C = alpha * B * A + beta * C (side = 1), \n
         *  where A is a hermitian matrix with upper (uplo = 0) of lower (uplo = 1) triangular part defined.
         */
        template<typename T>
        static void hemm(int side, int uplo, ftn_int m, ftn_int n, T alpha, T* A, ftn_len lda,
                         T* B, ftn_len ldb, T beta, T* C, ftn_len ldc);

        template<typename T>
        static void hemm(int side, int uplo, ftn_int m, ftn_int n, T alpha, matrix<T>& A,
                         matrix<T>& B, T beta, matrix<T>& C)
        {
            hemm(side, uplo, m, n, alpha, A.at(memory_t::host), A.ld(), B.at(memory_t::host), B.ld(), beta, C.at(memory_t::host), C.ld());
        }

        /// General matrix-matrix multiplication.
        /** Compute C = alpha * op(A) * op(B) + beta * op(C) with raw pointers. */
        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, T alpha, T const* A, ftn_int lda,
                         T const* B, ftn_int ldb, T beta, T* C, ftn_int ldc);

        /// Compute C = op(A) * op(B) operation with raw pointers.
        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, T const* A, ftn_int lda, T const* B, ftn_int ldb,
                         T* C, ftn_int ldc)
        {
            auto one = linalg_const<T>::one();
            auto zero = linalg_const<T>::zero();
            gemm(transa, transb, m, n, k, one, A, lda, B, ldb, zero, C, ldc);
        }

        /// Compute C = alpha * op(A) * op(B) + beta * op(C) with matrix objects.
        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, T alpha, matrix<T> const& A, matrix<T> const& B,
                         T beta, matrix<T>& C)
        {
            gemm(transa, transb, m, n, k, alpha, A.at(memory_t::host), A.ld(), B.at(memory_t::host), B.ld(), beta, C.at(memory_t::host), C.ld());
        }

        /// Compute C = op(A) * op(B) operation with matrix objects.
        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, matrix<T> const& A, matrix<T> const& B,
                         matrix<T>& C)
        {
            gemm(transa, transb, m, n, k, A.at(memory_t::host), A.ld(), B.at(memory_t::host), B.ld(), C.at(memory_t::host), C.ld());
        }

        /// Compute C = alpha * op(A) * op(B) + beta * op(C), generic interface
        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, T alpha,
                         dmatrix<T>& A, ftn_int ia, ftn_int ja, dmatrix<T>& B, ftn_int ib, ftn_int jb, T beta,
                         dmatrix<T>& C, ftn_int ic, ftn_int jc);

        /// Compute C = alpha * op(A) * op(B) + beta * op(C), simple interface - matrices start from (0, 0) corner.
        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                         T alpha, dmatrix<T>& A, dmatrix<T>& B, T beta, dmatrix<T>& C)
        {
            gemm(transa, transb, m, n, k, alpha, A, 0, 0, B, 0, 0, beta, C, 0, 0);
        }

        /// Compute the solution to system of linear equations A * X = B for GT matrices.
        template <typename T>
        static ftn_int gtsv(ftn_int n, ftn_int nrhs, T* dl, T* d, T* du, T* b, ftn_int ldb);

        /// Compute the solution to system of linear equations A * X = B for GE matrices.
        template <typename T>
        static ftn_int gesv(ftn_int n, ftn_int nrhs, T* A, ftn_int lda, T* B, ftn_int ldb);

        /// LU factorization
        template <typename T>
        static ftn_int getrf(ftn_int m, ftn_int n, T* A, ftn_int lda, ftn_int* ipiv);

        /// U*D*U^H factorization of hermitian matrix
        template <typename T>
        static ftn_int hetrf(ftn_int n, T* A, ftn_int lda, ftn_int* ipiv);

        template <typename T>
        static ftn_int getri(ftn_int n, T* A, ftn_int lda, ftn_int* ipiv);

        template <typename T>
        static ftn_int hetri(ftn_int n, T* A, ftn_int lda, ftn_int* ipiv);

        /// Invert a general matrix.
        template <typename T>
        static void geinv(ftn_int n, matrix<T>& A);

        /// Invert a general distributed matrix.
        template <typename T>
        static void geinv(ftn_int n, dmatrix<T>& A);

        template <typename T>
        static ftn_int sytrf(ftn_int n, T* A, ftn_int lda, ftn_int* ipiv);

        template <typename T>
        static ftn_int sytri(ftn_int n, T* A, ftn_int lda, ftn_int* ipiv);

        template <typename T>
        static void syinv(ftn_int n, matrix<T>& A);

        /// Invert a hermitian matrix.
        template <typename T>
        static void heinv(ftn_int n, matrix<T>& A);

        /// Cholesky factorization
        template <typename T>
        static ftn_int potrf(ftn_int n, T* A, ftn_int lda);

        template <typename T>
        static ftn_int potrf(ftn_int n, dmatrix<T>& A);

        /// Inversion of triangular matrix.
        template <typename T>
        static ftn_int trtri(ftn_int n, T* A, ftn_int lda);

        template <typename T>
        static ftn_int trtri(ftn_int n, dmatrix<T>& A);

        template <typename T>
        static void trmm(char side, char uplo, char transa, ftn_int m, ftn_int n, T aplha, T* A, ftn_int lda, T* B, ftn_int ldb);

        template <typename T>
        static ftn_int getrf(ftn_int m, ftn_int n, dmatrix<T>& A, ftn_int ia, ftn_int ja, ftn_int* ipiv);

        template <typename T>
        static ftn_int getri(ftn_int n, dmatrix<T>& A, ftn_int ia, ftn_int ja, ftn_int* ipiv);

        /// Conjugate transponse of the sub-matrix.
        /** \param [in] m Number of rows of the target sub-matrix.
         *  \param [in] n Number of columns of the target sub-matrix.
         */
        template <typename T>
        static void tranc(ftn_int m, ftn_int n, dmatrix<T>& A, ftn_int ia, ftn_int ja, dmatrix<T>& C, ftn_int ic, ftn_int jc);

        template <typename T>
        static void tranu(ftn_int m, ftn_int n, dmatrix<T>& A, ftn_int ia, ftn_int ja, dmatrix<T>& C, ftn_int ic, ftn_int jc);

        template <typename T>
        static void gemr2d(ftn_int m, ftn_int n, dmatrix<T>& A, ftn_int ia, ftn_int ja,
                           dmatrix<T>& B, ftn_int ib, ftn_int jb, ftn_int gcontext);

        template <typename T>
        static void geqrf(ftn_int m, ftn_int n, dmatrix<T>& A, ftn_int ia, ftn_int ja);
};

#ifdef __GPU
template<>
class linalg<GPU>: public linalg_base
{
    public:

        template<typename T>
        static void gemv(int trans, ftn_int m, ftn_int n, T* alpha, T* A, ftn_int lda, T* x, ftn_int incx,
                         T* beta, T* y, ftn_int incy, int stream_id);

        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, T const* alpha, T const* A, ftn_int lda,
                         T const* B, ftn_int ldb, T const* beta, T* C, ftn_int ldc, int stream_id = -1);

        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, T const* A, ftn_int lda,
                         T const* B, ftn_int ldb, T* C, ftn_int ldc, int stream_id = -1)
        {
            T const& alpha = linalg_const<T>::one();
            T const& beta = linalg_const<T>::zero();
            gemm(transa, transb, m, n, k, const_cast<T*>(&alpha), A, lda, B, ldb, const_cast<T*>(&beta), C, ldc, stream_id);
        }

        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, matrix<T> const& A, matrix<T> const& B,
                         matrix<T>& C, int stream_id = -1)
        {
            gemm(transa, transb, m, n, k, A.at(memory_t::device), A.ld(), B.at(memory_t::device), B.ld(), C.at(memory_t::device), C.ld(), stream_id);
        }

        template <typename T>
        static void gemm(int transa, int transb, ftn_int m, ftn_int n, ftn_int k, const T *alpha, matrix<T> const& A, matrix<T> const& B, const T *beta,
                         matrix<T>& C, int stream_id = -1)
        {
            gemm(transa, transb, m, n, k, alpha, A.at(memory_t::device), A.ld(), B.at(memory_t::device), B.ld(), beta, C.at(memory_t::device), C.ld(), stream_id);
        }

        /// Cholesky factorization
        template <typename T>
        static ftn_int potrf(ftn_int n, T* A, ftn_int lda);

        /// Inversion of triangular matrix.
        template <typename T>
        static ftn_int trtri(ftn_int n, T* A, ftn_int lda);

        template <typename T>
        static void trmm(char side, char uplo, char transa, ftn_int m, ftn_int n, T* aplha, T* A, ftn_int lda, T* B, ftn_int ldb);

        template<typename T>
        static void ger(ftn_int m, ftn_int n, T const* alpha, T* x, ftn_int incx, T* y, ftn_int incy, T* A, ftn_int lda, int stream_id = -1);

        template <typename T>
        static void axpy(int n__, T const* alpha__, T const* x__, int incx__, T* y__, int incy__);
};
#endif

// C = alpha * op(A) * op(B) + beta * op(C), double
template<>
inline void linalg<CPU>::gemm<ftn_double>(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                                          ftn_double alpha,
                                          ftn_double const* A, ftn_int lda,
                                          ftn_double const* B, ftn_int ldb,
                                          ftn_double beta,
                                          ftn_double* C, ftn_int ldc)
{
    assert(lda > 0);
    assert(ldb > 0);
    assert(ldc > 0);
    assert(m > 0);
    assert(n > 0);
    assert(k > 0);

    const char *trans[] = {"N", "T", "C"};

    FORTRAN(dgemm)(trans[transa], trans[transb], &m, &n, &k, &alpha, const_cast<ftn_double*>(A), &lda, const_cast<ftn_double*>(B), &ldb, &beta, C, &ldc,
                   (ftn_len)1, (ftn_len)1);
}

// C = alpha * op(A) * op(B) + beta * op(C), double_complex
template<>
inline void linalg<CPU>::gemm<ftn_double_complex>(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                                                  ftn_double_complex alpha,
                                                  ftn_double_complex const* A, ftn_int lda,
                                                  ftn_double_complex const* B, ftn_int ldb,
                                                  ftn_double_complex beta,
                                                  ftn_double_complex* C, ftn_int ldc)
{
    assert(lda > 0);
    assert(ldb > 0);
    assert(ldc > 0);
    assert(m > 0);
    assert(n > 0);
    assert(k > 0);

    const char *trans[] = {"N", "T", "C"};

    FORTRAN(zgemm)(trans[transa], trans[transb], &m, &n, &k, &alpha, const_cast<ftn_double_complex*>(A), &lda, const_cast<ftn_double_complex*>(B), &ldb, &beta, C, &ldc,
                   (ftn_len)1, (ftn_len)1);
}


template<>
inline void linalg<CPU>::gemv<ftn_double_complex>(int trans,
                                                  ftn_int m,
                                                  ftn_int n,
                                                  ftn_double_complex alpha,
                                                  ftn_double_complex const* A,
                                                  ftn_int lda,
                                                  ftn_double_complex const* x,
                                                  ftn_int incx,
                                                  ftn_double_complex beta,
                                                  ftn_double_complex* y,
                                                  ftn_int incy)
{
    const char *trans_c[] = {"N", "T", "C"};

    FORTRAN(zgemv)(trans_c[trans], &m, &n, &alpha, const_cast<ftn_double_complex*>(A), &lda, const_cast<ftn_double_complex*>(x), &incx, &beta, y, &incy, 1);
}

template<>
inline void linalg<CPU>::gemv<ftn_double>(int trans,
                                          ftn_int m,
                                          ftn_int n,
                                          ftn_double alpha,
                                          ftn_double const* A,
                                          ftn_int lda,
                                          ftn_double const* x,
                                          ftn_int incx,
                                          ftn_double beta,
                                          ftn_double* y,
                                          ftn_int incy)
{
    const char *trans_c[] = {"N", "T", "C"};

    FORTRAN(dgemv)(trans_c[trans], &m, &n, &alpha, const_cast<ftn_double*>(A), &lda, const_cast<ftn_double*>(x), &incx, &beta, y, &incy, 1);
}

template<>
inline void linalg<CPU>::ger<ftn_double_complex>(ftn_int             m,
                                                 ftn_int             n,
                                                 ftn_double_complex  alpha,
                                                 ftn_double_complex* x,
                                                 ftn_int             incx,
                                                 ftn_double_complex* y,
                                                 ftn_int             incy,
                                                 ftn_double_complex* A,
                                                 ftn_int             lda)
{
    FORTRAN(zgeru)(&m, &n, &alpha, x, &incx, y, &incy, A, &lda);
}

template<>
inline void linalg<CPU>::ger<ftn_double>(ftn_int     m,
                                         ftn_int     n,
                                         ftn_double  alpha,
                                         ftn_double* x,
                                         ftn_int     incx,
                                         ftn_double* y,
                                         ftn_int     incy,
                                         ftn_double* A,
                                         ftn_int     lda)
{
    FORTRAN(dger)(&m, &n, &alpha, x, &incx, y, &incy, A, &lda);
}

template<>
inline void linalg<CPU>::hemm<ftn_double_complex>(int side, int uplo, ftn_int m, ftn_int n, ftn_double_complex alpha,
                                                  ftn_double_complex* A, ftn_int lda, ftn_double_complex* B, ftn_int ldb,
                                                  ftn_double_complex beta, ftn_double_complex* C, ftn_int ldc)
{
    const char *sidestr[] = {"L", "R"};
    const char *uplostr[] = {"U", "L"};
    FORTRAN(zhemm)(sidestr[side], uplostr[uplo], &m, &n, &alpha, A, &lda, B, &ldb, &beta, C, &ldc, (ftn_len)1,
                   (ftn_len)1);
}

// LU factorization, double
template<>
inline ftn_int linalg<CPU>::getrf<ftn_double>(ftn_int m, ftn_int n, ftn_double* A, ftn_int lda, ftn_int* ipiv)
{
    ftn_int info;
    FORTRAN(dgetrf)(&m, &n, A, &lda, ipiv, &info);
    return info;
}

// LU factorization, double_complex
template<>
inline ftn_int linalg<CPU>::getrf<ftn_double_complex>(ftn_int m, ftn_int n, ftn_double_complex* A, ftn_int lda, ftn_int* ipiv)
{
    ftn_int info;
    FORTRAN(zgetrf)(&m, &n, A, &lda, ipiv, &info);
    return info;
}

// Inversion of LU factorized matrix, double
template<>
inline ftn_int linalg<CPU>::getri<ftn_double>(ftn_int n, ftn_double* A, ftn_int lda, ftn_int* ipiv)
{
    ftn_int nb = ilaenv(1, "dgetri", "U", n, -1, -1, -1);
    ftn_int lwork = n * nb;
    std::vector<ftn_double> work(lwork);

    int32_t info;
    FORTRAN(dgetri)(&n, A, &lda, ipiv, &work[0], &lwork, &info);
    return info;
}

// Inversion of LU factorized matrix, double_complex
template<>
inline ftn_int linalg<CPU>::getri<ftn_double_complex>(ftn_int n, ftn_double_complex* A, ftn_int lda, ftn_int* ipiv)
{
    ftn_int nb = ilaenv(1, "zgetri", "U", n, -1, -1, -1);
    ftn_int lwork = n * nb;
    std::vector<ftn_double_complex> work(lwork);

    int32_t info;
    FORTRAN(zgetri)(&n, A, &lda, ipiv, &work[0], &lwork, &info);
    return info;
}

// Inversion of general matrix, double
template <>
inline void linalg<CPU>::geinv<ftn_double>(ftn_int n, matrix<ftn_double>& A)
{
    std::vector<int> ipiv(n);
    int info = getrf(n, n, A.at(memory_t::host), A.ld(), &ipiv[0]);
    if (info)
    {
        printf("getrf returned %i\n", info);
        exit(-1);
    }

    info = getri(n, A.at(memory_t::host), A.ld(), &ipiv[0]);
    if (info)
    {
        printf("getri returned %i\n", info);
        exit(-1);
    }
}

// Inversion of general matrix, double_complex
template <>
inline void linalg<CPU>::geinv<ftn_double_complex>(ftn_int n, matrix<ftn_double_complex>& A)
{
    std::vector<int> ipiv(n);
    int info = getrf(n, n, A.at(memory_t::host), A.ld(), &ipiv[0]);
    if (info)
    {
        printf("getrf returned %i\n", info);
        exit(-1);
    }

    info = getri(n, A.at(memory_t::host), A.ld(), &ipiv[0]);
    if (info)
    {
        printf("getri returned %i\n", info);
        exit(-1);
    }
}

template<>
inline ftn_int linalg<CPU>::hetrf<ftn_double_complex>(ftn_int n, ftn_double_complex* A, ftn_int lda, ftn_int* ipiv)
{
    ftn_int nb = ilaenv(1, "zhetrf", "U", n, -1, -1, -1);
    ftn_int lwork = n * nb;
    std::vector<ftn_double_complex> work(lwork);

    ftn_int info;
    FORTRAN(zhetrf)("U", &n, A, &lda, ipiv, &work[0], &lwork, &info, (ftn_len)1);
    return info;
}

template<>
inline ftn_int linalg<CPU>::hetri<ftn_double_complex>(ftn_int n, ftn_double_complex* A, ftn_int lda, ftn_int* ipiv)
{
    std::vector<ftn_double_complex> work(n);
    ftn_int info;
    FORTRAN(zhetri)("U", &n, A, &lda, ipiv, &work[0], &info, (ftn_len)1);
    return info;
}

// Inversion of hermitian matrix, double_complex
template <>
inline void linalg<CPU>::heinv<ftn_double_complex>(ftn_int n, matrix<ftn_double_complex>& A)
{
    std::vector<int> ipiv(n);
    int info = hetrf(n, A.at(memory_t::host), A.ld(), &ipiv[0]);
    if (info) {
        printf("hetrf returned %i\n", info);
        exit(-1);
    }

    info = hetri(n, A.at(memory_t::host), A.ld(), &ipiv[0]);
    if (info) {
        printf("hetri returned %i\n", info);
        exit(-1);
    }
}

template<>
inline ftn_int linalg<CPU>::sytrf<ftn_double>(ftn_int n, ftn_double* A, ftn_int lda, ftn_int* ipiv)
{
    ftn_int nb = ilaenv(1, "dsytrf", "U", n, -1, -1, -1);
    ftn_int lwork = n * nb;
    std::vector<ftn_double> work(lwork);

    ftn_int info;
    FORTRAN(dsytrf)("U", &n, A, &lda, ipiv, &work[0], &lwork, &info, (ftn_len)1);
    return info;
}

template<>
inline ftn_int linalg<CPU>::sytri<ftn_double>(ftn_int n, ftn_double* A, ftn_int lda, ftn_int* ipiv)
{
    std::vector<ftn_double> work(n);
    ftn_int info;
    FORTRAN(dsytri)("U", &n, A, &lda, ipiv, &work[0], &info, (ftn_len)1);
    return info;
}

template <>
inline void linalg<CPU>::syinv<ftn_double>(ftn_int n, matrix<ftn_double>& A)
{
    std::vector<int> ipiv(n);
    int info = sytrf(n, A.at(memory_t::host), A.ld(), &ipiv[0]);
    if (info)
    {
        printf("sytrf returned %i\n", info);
        exit(-1);
    }

    info = sytri(n, A.at(memory_t::host), A.ld(), &ipiv[0]);
    if (info)
    {
        printf("sytri returned %i\n", info);
        exit(-1);
    }
}

template<>
inline ftn_int linalg<CPU>::gesv<ftn_double>(ftn_int n, ftn_int nrhs, ftn_double* A, ftn_int lda, ftn_double* B, ftn_int ldb)
{
    ftn_int info;
    std::vector<ftn_int> ipiv(n);
    FORTRAN(dgesv)(&n, &nrhs, A, &lda, &ipiv[0], B, &ldb, &info);
    return info;
}

template<>
inline ftn_int linalg<CPU>::gesv<ftn_double_complex>(ftn_int n, ftn_int nrhs, ftn_double_complex* A, ftn_int lda,
                                                     ftn_double_complex* B, ftn_int ldb)
{
    ftn_int info;
    std::vector<ftn_int> ipiv(n);
    FORTRAN(zgesv)(&n, &nrhs, A, &lda, &ipiv[0], B, &ldb, &info);
    return info;
}

template<>
inline ftn_int linalg<CPU>::gtsv<ftn_double>(ftn_int n, ftn_int nrhs, ftn_double* dl, ftn_double* d, ftn_double* du,
                                             ftn_double* b, ftn_int ldb)
{
    ftn_int info;
    FORTRAN(dgtsv)(&n, &nrhs, dl, d, du, b, &ldb, &info);
    return info;
}

template<>
inline ftn_int linalg<CPU>::gtsv<ftn_double_complex>(ftn_int n, ftn_int nrhs, ftn_double_complex* dl, ftn_double_complex* d,
                                                     ftn_double_complex* du, ftn_double_complex* b, ftn_int ldb)
{
    ftn_int info;
    FORTRAN(zgtsv)(&n, &nrhs, dl, d, du, b, &ldb, &info);
    return info;
}

template<>
inline ftn_int linalg<CPU>::potrf<ftn_double>(ftn_int n, ftn_double* A, ftn_int lda)
{
    ftn_int info;
    FORTRAN(dpotrf)("U", &n, A, &lda, &info, (ftn_len)1);
    return info;
}

template<>
inline ftn_int linalg<CPU>::potrf<ftn_double_complex>(ftn_int n, ftn_double_complex* A, ftn_int lda)
{
    ftn_int info;
    FORTRAN(zpotrf)("U", &n, A, &lda, &info, (ftn_len)1);
    return info;
}

template <>
inline ftn_int linalg<CPU>::trtri<ftn_double>(ftn_int n, ftn_double* A, ftn_int lda)
{
    ftn_int info;
    FORTRAN(dtrtri)("U", "N", &n, A, &lda, &info, (ftn_len)1, (ftn_len)1);
    return info;
}

template <>
inline ftn_int linalg<CPU>::trtri<ftn_double_complex>(ftn_int n, ftn_double_complex* A, ftn_int lda)
{
    ftn_int info;
    FORTRAN(ztrtri)("U", "N", &n, A, &lda, &info, (ftn_len)1, (ftn_len)1);
    return info;
}

template <>
inline void linalg<CPU>::trmm<ftn_double>(char side, char uplo, char transa, ftn_int m, ftn_int n, ftn_double alpha,
                                          ftn_double* A, ftn_int lda, ftn_double* B, ftn_int ldb)
{
    FORTRAN(dtrmm)(&side, &uplo, &transa, "N", &m, &n, &alpha, A, &lda, B, &ldb, (ftn_len)1, (ftn_len)1, (ftn_len)1, (ftn_len)1);
}

template <>
inline void linalg<CPU>::trmm<ftn_double_complex>(char side, char uplo, char transa, ftn_int m, ftn_int n, ftn_double_complex alpha,
                                                  ftn_double_complex* A, ftn_int lda, ftn_double_complex* B, ftn_int ldb)
{
    FORTRAN(ztrmm)(&side, &uplo, &transa, "N", &m, &n, &alpha, A, &lda, B, &ldb, (ftn_len)1, (ftn_len)1, (ftn_len)1, (ftn_len)1);
}

#ifdef __SCALAPACK
template<>
inline ftn_int linalg<CPU>::getrf<ftn_double_complex>(ftn_int m, ftn_int n, dmatrix<ftn_double_complex>& A,
                                                      ftn_int ia, ftn_int ja, ftn_int* ipiv)
{
    ftn_int info;
    ia++;
    ja++;
    FORTRAN(pzgetrf)(&m, &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), ipiv, &info);
    return info;
}

template<>
inline ftn_int linalg<CPU>::getri<ftn_double_complex>(ftn_int n, dmatrix<ftn_double_complex>& A, ftn_int ia, ftn_int ja,
                                                      ftn_int* ipiv)
{
    ftn_int info;
    ia++;
    ja++;


    ftn_int lwork, liwork, i;
    ftn_double_complex z;
    i = -1;
    /* query work sizes */
    FORTRAN(pzgetri)(&n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), &ipiv[0], &z, &i, &liwork, &i, &info);

    lwork = (int)real(z) + 1;
    std::vector<ftn_double_complex> work(lwork);
    std::vector<ftn_int> iwork(liwork);

    FORTRAN(pzgetri)(&n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), &ipiv[0], &work[0], &lwork, &iwork[0], &liwork, &info);

    return info;
}

template<>
inline void linalg<CPU>::geinv<ftn_double_complex>(ftn_int n, dmatrix<ftn_double_complex>& A)
{
    std::vector<ftn_int> ipiv(A.num_rows_local() + A.bs_row());
    ftn_int info = getrf(n, n, A, 0, 0, &ipiv[0]);
    if (info) {
        printf("getrf returned %i\n", info);
        exit(-1);
    }

    info = getri(n, A, 0, 0, &ipiv[0]);
    if (info) {
        printf("getri returned %i\n", info);
        exit(-1);
    }
}

template <>
inline void linalg<CPU>::tranc<ftn_double_complex>(ftn_int m, ftn_int n, dmatrix<ftn_double_complex>& A, ftn_int ia, ftn_int ja,
                                                   dmatrix<ftn_double_complex>& C, ftn_int ic, ftn_int jc)
{
    ia++; ja++;
    ic++; jc++;

    ftn_double_complex one = 1;
    ftn_double_complex zero = 0;

    pztranc(m, n, one, A.at(memory_t::host), ia, ja, A.descriptor(), zero, C.at(memory_t::host), ic, jc, C.descriptor());
}

template <>
inline void linalg<CPU>::tranu<ftn_double_complex>(ftn_int m, ftn_int n, dmatrix<ftn_double_complex>& A, ftn_int ia, ftn_int ja,
                                                   dmatrix<ftn_double_complex>& C, ftn_int ic, ftn_int jc)
{
    ia++; ja++;
    ic++; jc++;

    ftn_double_complex one = 1;
    ftn_double_complex zero = 0;

    pztranu(m, n, one, A.at(memory_t::host), ia, ja, A.descriptor(), zero, C.at(memory_t::host), ic, jc, C.descriptor());
}

template <>
inline void linalg<CPU>::tranc<ftn_double>(ftn_int m, ftn_int n, dmatrix<ftn_double>& A, ftn_int ia, ftn_int ja,
                                           dmatrix<ftn_double>& C, ftn_int ic, ftn_int jc)
{
    ia++; ja++;
    ic++; jc++;

    ftn_double one = 1;
    ftn_double zero = 0;

    pdtran(m, n, one, A.at(memory_t::host), ia, ja, A.descriptor(), zero, C.at(memory_t::host), ic, jc, C.descriptor());
}

template <>
inline void linalg<CPU>::gemr2d(ftn_int m, ftn_int n, dmatrix<ftn_double_complex>& A, ftn_int ia, ftn_int ja,
                                dmatrix<ftn_double_complex>& B, ftn_int ib, ftn_int jb, ftn_int gcontext)
{
    ia++; ja++;
    ib++; jb++;
    FORTRAN(pzgemr2d)(&m, &n, A.at(memory_t::host), &ia, &ja, A.descriptor(), B.at(memory_t::host), &ib, &jb, B.descriptor(), &gcontext);
}

template<>
inline void linalg<CPU>::gemm<ftn_double>(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                                          ftn_double alpha, dmatrix<ftn_double>& A, ftn_int ia, ftn_int ja,
                                          dmatrix<ftn_double>& B, ftn_int ib, ftn_int jb, ftn_double beta,
                                          dmatrix<ftn_double>& C, ftn_int ic, ftn_int jc)
{
    assert(A.ld() != 0);
    assert(B.ld() != 0);
    assert(C.ld() != 0);

    const char *trans[] = {"N", "T", "C"};

    ia++; ja++;
    ib++; jb++;
    ic++; jc++;
    FORTRAN(pdgemm)(trans[transa], trans[transb], &m, &n, &k, &alpha, A.at(memory_t::host), &ia, &ja, A.descriptor(),
                    B.at(memory_t::host), &ib, &jb, B.descriptor(), &beta, C.at(memory_t::host), &ic, &jc, C.descriptor(),
                    (ftn_len)1, (ftn_len)1);
}

template<>
inline void linalg<CPU>::gemm<ftn_double_complex>(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                                                  ftn_double_complex alpha,
                                                  dmatrix<ftn_double_complex>& A, ftn_int ia, ftn_int ja,
                                                  dmatrix<ftn_double_complex>& B, ftn_int ib, ftn_int jb,
                                                  ftn_double_complex beta,
                                                  dmatrix<ftn_double_complex>& C, ftn_int ic, ftn_int jc)
{
    assert(A.ld() != 0);
    assert(B.ld() != 0);
    assert(C.ld() != 0);

    const char *trans[] = {"N", "T", "C"};

    ia++; ja++;
    ib++; jb++;
    ic++; jc++;
    FORTRAN(pzgemm)(trans[transa], trans[transb], &m, &n, &k, &alpha, A.at(memory_t::host), &ia, &ja, A.descriptor(),
                    B.at(memory_t::host), &ib, &jb, B.descriptor(), &beta, C.at(memory_t::host), &ic, &jc, C.descriptor(),
                    (ftn_len)1, (ftn_len)1);
}

template<>
inline ftn_int linalg<CPU>::potrf<ftn_double>(ftn_int n, dmatrix<ftn_double>& A)
{
    ftn_int ia{1};
    ftn_int ja{1};
    ftn_int info;
    FORTRAN(pdpotrf)("U", &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), &info, (ftn_len)1);
    return info;
}

template<>
inline ftn_int linalg<CPU>::potrf<ftn_double_complex>(ftn_int n, dmatrix<ftn_double_complex>& A)
{
    ftn_int ia{1};
    ftn_int ja{1};
    ftn_int info;
    FORTRAN(pzpotrf)("U", &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), &info, (ftn_len)1);
    return info;
}

template<>
inline ftn_int linalg<CPU>::trtri<ftn_double>(ftn_int n, dmatrix<ftn_double>& A)
{
    ftn_int ia{1};
    ftn_int ja{1};
    ftn_int info;
    FORTRAN(pdtrtri)("U", "N", &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), &info, (ftn_len)1, (ftn_len)1);
    return info;
}

template<>
inline ftn_int linalg<CPU>::trtri<ftn_double_complex>(ftn_int n, dmatrix<ftn_double_complex>& A)
{
    ftn_int ia{1};
    ftn_int ja{1};
    ftn_int info;
    FORTRAN(pztrtri)("U", "N", &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), &info, (ftn_len)1, (ftn_len)1);
    return info;
}

template <>
inline void linalg<CPU>::geqrf<ftn_double_complex>(ftn_int m, ftn_int n, dmatrix<ftn_double_complex>& A, ftn_int ia, ftn_int ja)
{
    ia++; ja++;
    ftn_int lwork = -1;
    ftn_double_complex z;
    ftn_int info;
    FORTRAN(pzgeqrf)(&m, &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), &z, &z, &lwork, &info);
    lwork = static_cast<int>(z.real() + 1);
    std::vector<ftn_double_complex> work(lwork);
    std::vector<ftn_double_complex> tau(std::max(m, n));
    FORTRAN(pzgeqrf)(&m, &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), tau.data(), work.data(), &lwork, &info);
}

template <>
inline void linalg<CPU>::geqrf<ftn_double>(ftn_int m, ftn_int n, dmatrix<ftn_double>& A, ftn_int ia, ftn_int ja)
{
    ia++; ja++;
    ftn_int lwork = -1;
    ftn_double z;
    ftn_int info;
    FORTRAN(pdgeqrf)(&m, &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), &z, &z, &lwork, &info);
    lwork = static_cast<int>(z + 1);
    std::vector<ftn_double> work(lwork);
    std::vector<ftn_double> tau(std::max(m, n));
    FORTRAN(pdgeqrf)(&m, &n, A.at(memory_t::host), &ia, &ja, const_cast<int*>(A.descriptor()), tau.data(), work.data(), &lwork, &info);
}

#else
template<>
inline ftn_int linalg<CPU>::potrf<ftn_double>(ftn_int n, dmatrix<ftn_double>& A)
{
    return linalg<CPU>::potrf<ftn_double>(n, A.at(memory_t::host), A.ld());
}

template<>
inline ftn_int linalg<CPU>::potrf<ftn_double_complex>(ftn_int n, dmatrix<ftn_double_complex>& A)
{
    return linalg<CPU>::potrf<ftn_double_complex>(n, A.at(memory_t::host), A.ld());
}

template<>
inline ftn_int linalg<CPU>::trtri<ftn_double>(ftn_int n, dmatrix<ftn_double>& A)
{
    return linalg<CPU>::trtri<ftn_double>(n, A.at(memory_t::host), A.ld());
}

template<>
inline ftn_int linalg<CPU>::trtri<ftn_double_complex>(ftn_int n, dmatrix<ftn_double_complex>& A)
{
    return linalg<CPU>::trtri<ftn_double_complex>(n, A.at(memory_t::host), A.ld());
}

template <>
inline void linalg<CPU>::geqrf<ftn_double_complex>(ftn_int m, ftn_int n, dmatrix<ftn_double_complex>& A, ftn_int ia, ftn_int ja)
{
    ftn_int lwork = -1;
    ftn_double_complex z;
    ftn_int info;
    ftn_int lda = A.ld();
    FORTRAN(zgeqrf)(&m, &n, A.at(memory_t::host, ia, ja), &lda, &z, &z, &lwork, &info);
    lwork = static_cast<int>(z.real() + 1);
    std::vector<ftn_double_complex> work(lwork);
    std::vector<ftn_double_complex> tau(std::max(m, n));
    FORTRAN(zgeqrf)(&m, &n, A.at(memory_t::host, ia, ja), &lda, tau.data(), work.data(), &lwork, &info);
}

template <>
inline void linalg<CPU>::geqrf<ftn_double>(ftn_int m, ftn_int n, dmatrix<ftn_double>& A, ftn_int ia, ftn_int ja)
{
    ftn_int lwork = -1;
    ftn_double z;
    ftn_int info;
    ftn_int lda = A.ld();
    FORTRAN(dgeqrf)(&m, &n, A.at(memory_t::host, ia, ja), &lda, &z, &z, &lwork, &info);
    lwork = static_cast<int>(z + 1);
    std::vector<ftn_double> work(lwork);
    std::vector<ftn_double> tau(std::max(m, n));
    FORTRAN(dgeqrf)(&m, &n, A.at(memory_t::host, ia, ja), &lda, tau.data(), work.data(), &lwork, &info);
}

template<>
inline void linalg<CPU>::gemm<ftn_double_complex>(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                                                  ftn_double_complex alpha,
                                                  dmatrix<ftn_double_complex>& A, ftn_int ia, ftn_int ja,
                                                  dmatrix<ftn_double_complex>& B, ftn_int ib, ftn_int jb,
                                                  ftn_double_complex beta,
                                                  dmatrix<ftn_double_complex>& C, ftn_int ic, ftn_int jc)
{
    gemm(transa, transb, m, n, k, alpha, A.at(memory_t::host, ia, ja), A.ld(), B.at(memory_t::host, ib, jb), B.ld(),
         beta, C.at(memory_t::host, ic, jc), C.ld());
}

template<>
inline void linalg<CPU>::gemm<ftn_double_complex>(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                                                  ftn_double_complex alpha,
                                                  dmatrix<ftn_double_complex>& A, dmatrix<ftn_double_complex>& B,
                                                  ftn_double_complex beta, dmatrix<ftn_double_complex>& C)
{
    gemm(transa, transb, m, n, k, alpha, A.at(memory_t::host), A.ld(), B.at(memory_t::host), B.ld(), beta, C.at(memory_t::host), C.ld());
}

template<>
inline void linalg<CPU>::gemm<ftn_double>(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                                          ftn_double alpha,
                                          dmatrix<ftn_double>& A, ftn_int ia, ftn_int ja,
                                          dmatrix<ftn_double>& B, ftn_int ib, ftn_int jb,
                                          ftn_double beta,
                                          dmatrix<ftn_double>& C, ftn_int ic, ftn_int jc)
{
    gemm(transa, transb, m, n, k, alpha, A.at(memory_t::host, ia, ja), A.ld(), B.at(memory_t::host, ib, jb), B.ld(),
         beta, C.at(memory_t::host, ic, jc), C.ld());
}

template<>
inline void linalg<CPU>::gemm<ftn_double>(int transa, int transb, ftn_int m, ftn_int n, ftn_int k,
                                          ftn_double alpha,
                                          dmatrix<ftn_double>& A, dmatrix<ftn_double>& B,
                                          ftn_double beta, dmatrix<ftn_double>& C)
{
    gemm(transa, transb, m, n, k, alpha, A.at(memory_t::host), A.ld(), B.at(memory_t::host), B.ld(), beta, C.at(memory_t::host), C.ld());
}
#endif

#if defined(__GPU) && defined(__CUDA)
template<>
inline void linalg<GPU>::gemv<ftn_double_complex>(int trans__, ftn_int m, ftn_int n, ftn_double_complex* alpha,
                                                  ftn_double_complex* A, ftn_int lda, ftn_double_complex* x, ftn_int incx,
                                                  ftn_double_complex* beta, ftn_double_complex* y, ftn_int incy,
                                                  int stream_id)
{
    const char trans[] = {'N', 'T', 'C'};
    cublas::zgemv(trans[trans__], m, n, (cuDoubleComplex*)alpha, (cuDoubleComplex*)A, lda, (cuDoubleComplex*)x, incx, (cuDoubleComplex*)beta, (cuDoubleComplex*)y, incy, stream_id);
}

// Generic interface to zgemm
template<>
inline void linalg<GPU>::gemm<ftn_double_complex>(int transa__, int transb__, ftn_int m, ftn_int n, ftn_int k,
                                                  ftn_double_complex const* alpha, ftn_double_complex const* A, ftn_int lda,
                                                  ftn_double_complex const* B, ftn_int ldb, ftn_double_complex const* beta,
                                                  ftn_double_complex* C, ftn_int ldc, int stream_id)
{
    assert(lda > 0);
    assert(ldb > 0);
    assert(ldc > 0);
    assert(m > 0);
    assert(n > 0);
    assert(k > 0);
    const char trans[] = {'N', 'T', 'C'};
    cublas::zgemm(trans[transa__], trans[transb__], m, n, k, (cuDoubleComplex*)alpha, (cuDoubleComplex*)A, lda, (cuDoubleComplex*)B, ldb, (cuDoubleComplex*)beta, (cuDoubleComplex*)C, ldc, stream_id);
}

// Generic interface to dgemm
template<>
inline void linalg<GPU>::gemm<ftn_double>(int transa__, int transb__, ftn_int m, ftn_int n, ftn_int k,
                                          ftn_double const* alpha, ftn_double const* A, ftn_int lda,
                                          ftn_double const* B, ftn_int ldb, ftn_double const* beta,
                                          ftn_double* C, ftn_int ldc, int stream_id)
{
    assert(lda > 0);
    assert(ldb > 0);
    assert(ldc > 0);
    assert(m > 0);
    assert(n > 0);
    assert(k > 0);
    const char trans[] = {'N', 'T', 'C'};
    cublas::dgemm(trans[transa__], trans[transb__], m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, stream_id);
}

template<>
inline ftn_int linalg<GPU>::potrf<ftn_double>(ftn_int n,
                                              ftn_double* A,
                                              ftn_int lda)
{
    #ifdef __MAGMA
    return magma::dpotrf('U', n, A, lda);
    #else
    printf("not compiled with MAGMA support\n");
    raise(SIGTERM);
    #endif
    return -1;
}

template<>
inline ftn_int linalg<GPU>::potrf<ftn_double_complex>(ftn_int n,
                                                      ftn_double_complex* A,
                                                      ftn_int lda)
{
    #ifdef __MAGMA
    return magma::zpotrf('U', n, (magmaDoubleComplex*)A, lda);
    #else
    printf("not compiled with MAGMA support\n");
    raise(SIGTERM);
    #endif
    return -1;
}

template <>
inline ftn_int linalg<GPU>::trtri<ftn_double>(ftn_int n,
                                              ftn_double* A,
                                              ftn_int lda)
{
    #ifdef __MAGMA
    return magma::dtrtri('U', n, A, lda);
    #else
    printf("not compiled with MAGMA support\n");
    raise(SIGTERM);
    #endif
    return -1;
}

template <>
inline ftn_int linalg<GPU>::trtri<ftn_double_complex>(ftn_int n,
                                                      ftn_double_complex* A,
                                                      ftn_int lda)
{
    #ifdef __MAGMA
    return magma::ztrtri('U', n, (magmaDoubleComplex*)A, lda);
    #else
    printf("not compiled with MAGMA support\n");
    raise(SIGTERM);
    #endif
    return -1;
}

template <>
inline void linalg<GPU>::trmm<ftn_double>(char side,
                                          char uplo,
                                          char transa,
                                          ftn_int m,
                                          ftn_int n,
                                          ftn_double* alpha,
                                          ftn_double* A,
                                          ftn_int lda,
                                          ftn_double* B,
                                          ftn_int ldb)
{
    cublas::dtrmm(side, uplo, transa, 'N', m, n, alpha, A, lda, B, ldb);
}

template <>
inline void linalg<GPU>::trmm<ftn_double_complex>(char side,
                                                  char uplo,
                                                  char transa,
                                                  ftn_int m,
                                                  ftn_int n,
                                                  ftn_double_complex* alpha,
                                                  ftn_double_complex* A,
                                                  ftn_int lda,
                                                  ftn_double_complex* B,
                                                  ftn_int ldb)
{
    cublas::ztrmm(side, uplo, transa, 'N', m, n, (cuDoubleComplex*)alpha, (cuDoubleComplex*)A, lda, (cuDoubleComplex*)B, ldb);
}

template<>
inline void linalg<GPU>::ger<ftn_double>(ftn_int     m,
                                         ftn_int     n,
                                         ftn_double const* alpha,
                                         ftn_double* x,
                                         ftn_int     incx,
                                         ftn_double* y,
                                         ftn_int     incy,
                                         ftn_double* A,
                                         ftn_int     lda,
                                         int         stream_id)
{
    cublas::dger(m, n, alpha, x, incx, y, incy, A, lda, stream_id);
}

template<>
inline void linalg<GPU>::ger<ftn_double_complex>(ftn_int             m,
                                                 ftn_int             n,
                                                 ftn_double_complex const* alpha,
                                                 ftn_double_complex* x,
                                                 ftn_int             incx,
                                                 ftn_double_complex* y,
                                                 ftn_int             incy,
                                                 ftn_double_complex* A,
                                                 ftn_int             lda,
                                                 int                 stream_id)
{
    cublas::zgeru(m, n, (cuDoubleComplex const*)alpha, (cuDoubleComplex*)x, incx, (cuDoubleComplex*)y, incy, (cuDoubleComplex*)A, lda, stream_id);
}

template <>
inline void linalg<GPU>::axpy<ftn_double_complex>(ftn_int n__,
                                                  ftn_double_complex const* alpha__,
                                                  ftn_double_complex const* x__,
                                                  ftn_int incx__,
                                                  ftn_double_complex* y__,
                                                  ftn_int incy__)
{
    cublas::zaxpy(n__, (cuDoubleComplex const*)alpha__, (cuDoubleComplex*)x__, incx__, (cuDoubleComplex*)y__, incy__);
}
#endif // __GPU

template <typename T>
inline void check_hermitian(const std::string& name, matrix<T> const& mtrx, int n = -1)
{
    assert(mtrx.size(0) == mtrx.size(1));

    double maxdiff = 0.0;
    int i0 = -1;
    int j0 = -1;

    if (n == -1) {
        n = static_cast<int>(mtrx.size(0));
    }

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double diff = std::abs(mtrx(i, j) - std::conj(mtrx(j, i)));
            if (diff > maxdiff) {
                maxdiff = diff;
                i0 = i;
                j0 = j;
            }
        }
    }

    if (maxdiff > 1e-10) {
        std::stringstream s;
        s << name << " is not a symmetric or hermitian matrix" << std::endl
          << "  maximum error: i, j : " << i0 << " " << j0 << " diff : " << maxdiff;

        WARNING(s);
    }
}

template <typename T>
inline double check_hermitian(dmatrix<T>& mtrx__, int n__)
{
    double max_diff{0};
#ifdef __SCALAPACK
    dmatrix<T> tmp(n__, n__, mtrx__.blacs_grid(), mtrx__.bs_row(), mtrx__.bs_col());
    linalg<CPU>::tranc(n__, n__, mtrx__, 0, 0, tmp, 0, 0);
    for (int i = 0; i < tmp.num_cols_local(); i++) {
        for (int j = 0; j < tmp.num_rows_local(); j++) {
            max_diff = std::max(max_diff, std::abs(mtrx__(j, i) - tmp(j, i)));
        }
    }
    mtrx__.blacs_grid().comm().template allreduce<double, mpi_op_t::max>(&max_diff, 1);
#else
    for (int i = 0; i < n__; i++) {
        for (int j = 0; j < n__; j++) {
            max_diff = std::max(max_diff, std::abs(mtrx__(j, i) - std::conj(mtrx__(i, j))));
        }
    }
#endif
    return max_diff;
}

template <typename T>
inline double check_identity(dmatrix<T>& mtrx__, int n__)
{
    double max_diff{0};
    for (int i = 0; i < mtrx__.num_cols_local(); i++) {
        int icol = mtrx__.icol(i);
        for (int j = 0; j < mtrx__.num_rows_local(); j++) {
            int jrow = mtrx__.irow(j);
            if (icol == jrow) {
                max_diff = std::max(max_diff, std::abs(mtrx__(j, i) - 1.0));
            } else {
                max_diff = std::max(max_diff, std::abs(mtrx__(j, i)));
            }
        }
    }
    mtrx__.comm().template allreduce<double, mpi_op_t::max>(&max_diff, 1);
    return max_diff;
}

class linalg2
{
  private:
    linalg_t la_;
  public:
    linalg2(linalg_t la__)
        : la_(la__)
    {
    }

    template <typename T>
    inline void gemm(char transa, char transb, ftn_int m, ftn_int n, ftn_int k, T const* alpha, T const* A, ftn_int lda,
                     T const* B, ftn_int ldb, T const* beta, T* C, ftn_int ldc, stream_id sid = stream_id(-1)) const;

    template<typename T>
    void ger(ftn_int m, ftn_int n, T const* alpha, T const* x, ftn_int incx, T const* y, ftn_int incy, T* A, ftn_int lda,
             stream_id sid = stream_id(-1)) const;

    template <typename T>
    void trmm(char side, char uplo, char transa, ftn_int m, ftn_int n, T const* aplha, T const* A, ftn_int lda, T* B, ftn_int ldb);
};

template <>
inline void linalg2::gemm<ftn_double>(char transa, char transb, ftn_int m, ftn_int n, ftn_int k, ftn_double const* alpha,
                                      ftn_double const* A, ftn_int lda, ftn_double const* B, ftn_int ldb,
                                      ftn_double const* beta, ftn_double* C, ftn_int ldc, stream_id sid) const
{
    assert(lda > 0);
    assert(ldb > 0);
    assert(ldc > 0);
    assert(m > 0);
    assert(n > 0);
    assert(k > 0);
    switch (la_) {
        case linalg_t::blas: {
            FORTRAN(dgemm)(&transa, &transb, &m, &n, &k, const_cast<double*>(alpha), const_cast<double*>(A), &lda,
                           const_cast<double*>(B), &ldb, const_cast<double*>(beta), C, &ldc, (ftn_len)1, (ftn_len)1);
            break;
        }
        case linalg_t::cublas: {
#if defined(__GPU) && defined(__CUDA)
            cublas::dgemm(transa, transb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc, sid());
#else
            throw std::runtime_error("not compiled with cublas");
#endif
            break;
        }
        case linalg_t::cublasxt: {
#if defined(__GPU) && defined(__CUDA)
            cublas::xt::dgemm(transa, transb, m, n, k, alpha, A, lda, B, ldb, beta, C, ldc);
#else
            throw std::runtime_error("not compiled with cublasxt");
#endif
            break;

        }
        default: {
            throw std::runtime_error("wrong type of linear algebra library");
            break;
        }
    }
}

template <>
inline void linalg2::gemm<ftn_double_complex>(char transa, char transb, ftn_int m, ftn_int n, ftn_int k,
                                              ftn_double_complex const* alpha, ftn_double_complex const* A, ftn_int lda,
                                              ftn_double_complex const* B, ftn_int ldb, ftn_double_complex const *beta,
                                              ftn_double_complex* C, ftn_int ldc, stream_id sid) const
{
    assert(lda > 0);
    assert(ldb > 0);
    assert(ldc > 0);
    assert(m > 0);
    assert(n > 0);
    assert(k > 0);
    switch (la_) {
        case linalg_t::blas: {
            FORTRAN(zgemm)(&transa, &transb, &m, &n, &k, const_cast<ftn_double_complex*>(alpha),
                           const_cast<ftn_double_complex*>(A), &lda, const_cast<ftn_double_complex*>(B), &ldb,
                           const_cast<ftn_double_complex*>(beta), C, &ldc, (ftn_len)1, (ftn_len)1);
            break;
        }
        case linalg_t::cublas: {
#if defined(__GPU) && defined(__CUDA)
            cublas::zgemm(transa, transb, m, n, k, reinterpret_cast<cuDoubleComplex const*>(alpha),
                          reinterpret_cast<cuDoubleComplex const*>(A), lda, reinterpret_cast<cuDoubleComplex const*>(B), 
                          ldb, reinterpret_cast<cuDoubleComplex const*>(beta),
                          reinterpret_cast<cuDoubleComplex*>(C), ldc, sid());
#else
            throw std::runtime_error("not compiled with cublas");
#endif
            break;

        }
        case linalg_t::cublasxt: {
#if defined(__GPU) && defined(__CUDA)
            cublas::xt::zgemm(transa, transb, m, n, k, reinterpret_cast<cuDoubleComplex const*>(alpha),
                              reinterpret_cast<cuDoubleComplex const*>(A), lda,
                              reinterpret_cast<cuDoubleComplex const*>(B), ldb,
                              reinterpret_cast<cuDoubleComplex const*>(beta),
                              reinterpret_cast<cuDoubleComplex*>(C), ldc);
#else
            throw std::runtime_error("not compiled with cublasxt");
#endif
            break;

        }
        default: {
            throw std::runtime_error("wrong type of linear algebra library");
            break;
        }
    }
}

template<>
inline void linalg2::ger<ftn_double>(ftn_int m, ftn_int n, ftn_double const* alpha, ftn_double const* x, ftn_int incx,
                                     ftn_double const* y, ftn_int incy, ftn_double* A, ftn_int lda, stream_id sid) const
{
    switch (la_) {
        case linalg_t::blas: {
            FORTRAN(dger)(&m, &n, const_cast<ftn_double*>(alpha), const_cast<ftn_double*>(x), &incx,
                          const_cast<ftn_double*>(y), &incy, A, &lda);
            break;
        }
        case  linalg_t::cublas: {
#if defined(__GPU) && defined(__CUDA)
            cublas::dger(m, n, alpha, x, incx, y, incy, A, lda, sid());
#else
            throw std::runtime_error("not compiled with cublas");
#endif
            break;
        }
        case linalg_t::cublasxt: {
            throw std::runtime_error("(d,z)ger is not implemented in cublasxt");
            break;
        }
        default: {
            throw std::runtime_error("wrong type of linear algebra library");
            break;
        }
    }
}

template <>
inline void linalg2::trmm<ftn_double>(char side, char uplo, char transa, ftn_int m, ftn_int n, ftn_double const* alpha,
                                      ftn_double const* A, ftn_int lda, ftn_double* B, ftn_int ldb)
{
    switch (la_) {
        case linalg_t::blas: {
            FORTRAN(dtrmm)(&side, &uplo, &transa, "N", &m, &n, const_cast<ftn_double*>(alpha),
                           const_cast<ftn_double*>(A), &lda, B, &ldb, (ftn_len)1, (ftn_len)1, (ftn_len)1, (ftn_len)1);
            break;
        }
        case  linalg_t::cublas: {
#if defined(__GPU) && defined(__CUDA)
            cublas::dtrmm(side, uplo, transa, 'N', m, n, alpha, A, lda, B, ldb);
#else
            throw std::runtime_error("not compiled with cublas");
#endif
            break;
        }
        case linalg_t::cublasxt: {
#if defined(__GPU) && defined(__CUDA)
            cublas::xt::dtrmm(side, uplo, transa, 'N', m, n, alpha, A, lda, B, ldb);
#else
            throw std::runtime_error("not compiled with cublasxt");
#endif
            break;
        }
        default: {
            throw std::runtime_error("wrong type of linear algebra library");
            break;
        }
    }
}

template <>
inline void linalg2::trmm<ftn_double_complex>(char side, char uplo, char transa, ftn_int m, ftn_int n,
                                              ftn_double_complex const* alpha, ftn_double_complex const* A,
                                              ftn_int lda, ftn_double_complex* B, ftn_int ldb)
{
    switch (la_) {
        case linalg_t::blas: {
            FORTRAN(ztrmm)(&side, &uplo, &transa, "N", &m, &n, const_cast<ftn_double_complex*>(alpha),
                           const_cast<ftn_double_complex*>(A), &lda, B, &ldb, (ftn_len)1, (ftn_len)1, (ftn_len)1, (ftn_len)1);
            break;
        }
        case  linalg_t::cublas: {
#if defined(__GPU) && defined(__CUDA)
            cublas::ztrmm(side, uplo, transa, 'N', m, n, reinterpret_cast<cuDoubleComplex const*>(alpha), 
                          reinterpret_cast<cuDoubleComplex const*>(A), lda, reinterpret_cast<cuDoubleComplex*>(B), ldb);
#else
            throw std::runtime_error("not compiled with cublas");
#endif
            break;
        }
        case linalg_t::cublasxt: {
#if defined(__GPU) && defined(__CUDA)
            cublas::xt::ztrmm(side, uplo, transa, 'N', m, n, reinterpret_cast<cuDoubleComplex const*>(alpha),
                              reinterpret_cast<cuDoubleComplex const*>(A), lda, reinterpret_cast<cuDoubleComplex*>(B), ldb);
#else
            throw std::runtime_error("not compiled with cublasxt");
#endif
            break;
        }
        default: {
            throw std::runtime_error("wrong type of linear algebra library");
            break;
        }
    }
}

} // namespace sddk

#endif // __LINALG_HPP__
