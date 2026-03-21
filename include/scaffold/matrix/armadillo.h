#ifndef SCAFFOLD_MATRIX_ARMADILLO_H_
#define SCAFFOLD_MATRIX_ARMADILLO_H_

#define ARMA_NO_DEBUG
#define ARMA_DONT_USE_WRAPPER
#define ARMA_USE_LAPACK
#define ARMA_USE_BLAS
#define ARMA_USE_CXX11

#include <armadillo>

namespace scaffold { namespace matrix {

// Basic types
template<typename T> using vec = arma::Col<T>;
template<typename T> using mat = arma::Mat<T>;
template<typename T> using cube = arma::Cube<T>;
template<typename T> using field = arma::field<T>;

// Initialization
template<typename T, typename... Params>
inline T identity(Params... parameters) { return arma::eye<T>(parameters...); }
using arma::ones;
template<typename T, typename... Params>
inline T random(Params... parameters) { return arma::randu<T>(parameters...); }
using arma::zeros;

// Functions
template <typename T>
auto sum(T&& val) -> decltype(arma::accu(std::forward<T>(val))) { return arma::accu(std::forward<T>(val)); }
using arma::det;
using arma::inv;
template<typename T>
auto mag(T&& val) -> decltype(arma::norm(std::forward<T>(val),2)) { return arma::norm(std::forward<T>(val),2); }
using arma::dot;
using arma::solve;

}} // namespace

#endif // SCAFFOLD_MATRIX_ARMADILLO_H_
