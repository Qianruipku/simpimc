#ifndef SCAFFOLD_MATRIX_EIGEN_H_
#define SCAFFOLD_MATRIX_EIGEN_H_

#define EIGEN_NO_DEBUG
#include <eigen3/Eigen/Eigen>

namespace scaffold { namespace matrix {

// Basic types
template<typename T> using vec = Eigen::Matrix<T, Eigen::Dynamic, 1>;
template<typename T> using mat = Eigen::Matrix<T, Eigen::Dynamic, Eigen::Dynamic>;
template<typename T>
struct cube {
    cube (int _n_rows, int _n_cols, int _n_slices)
      : n_slices(_n_slices), n_rows(_n_rows), n_cols(_n_cols), data(_n_rows,_n_slices*_n_cols)
    {}

    T& operator()(const int i, const int j, const int k)
    {
      return data(i,k*n_cols + j);
    }

    friend std::ostream& operator<<(std::ostream& out, cube<T> &val)
    {
      out << val.data;
      return out;
    }

    size_t size() { return n_slices*n_rows*n_cols; }

    mat<T> data;
    int n_slices, n_rows, n_cols;
};

template<typename T>
struct field {
    field () {}

    field (int _n_rows)
      : n_rows(_n_rows), n_cols(1), data_1d(_n_rows), dim(1)
    {}

    field (int _n_rows, int _n_cols)
      : n_rows(_n_rows), n_cols(_n_cols), data_2d(_n_rows), dim(2)
    {
      for (int i=0; i<n_rows; ++i)
        data_2d[i].resize(n_cols);
    }

    T& operator()(const int i)
    {
      return data_1d[i];
    }

    T& operator()(const int i, const int j)
    {
      return data_2d[i][j];
    }

    void set_size(const int _n_rows)
    {
      dim = 1;
      n_rows = _n_rows;
      data_1d.resize(n_rows);
    }

    void set_size(const int _n_rows, const int _n_cols)
    {
      dim = 2;
      n_rows = _n_rows;
      n_cols = _n_cols;
      data_2d.resize(n_rows);
      for (int i=0; i<n_rows; ++i)
        data_2d[i].resize(n_cols);
    }

    size_t size() { return n_rows*n_cols; }

    friend std::ostream& operator<<(std::ostream& out, field<T> &val)
    {
      for (int i=0; i<val.n_rows; ++i) {
        if (val.dim>1) {
          for (int j=0; j<val.n_cols; ++j) {
            out << val(i,j);
          }
        } else {
          out << val(i);
        }
      }
      return out;
    }

    std::vector<T> data_1d;
    std::vector< std::vector<T> > data_2d;
    int n_rows, n_cols, dim;
};

// Initialization
template<typename T>
inline T identity(int i, int j) { return T::Identity(i,j); }
template<typename T, typename... Params>
inline T ones(Params... parameters) { return T::Constant(parameters...,1); }
template<typename T, typename... Params>
inline T random(Params... parameters) { return T::Random(parameters...); }
template<typename T, typename... Params>
inline T zeros(Params... parameters) { return T::Zero(parameters...); }

// Functions
template <typename T>
auto sum(T&& val) -> decltype(std::forward<T>(val).sum()) { return std::forward<T>(val).sum(); }
template<typename T>
auto det(T&& val) -> decltype(std::forward<T>(val).determinant()) { return std::forward<T>(val).determinant(); }
template<typename T>
auto inv(T&& val) -> decltype(std::forward<T>(val).inverse()) { return std::forward<T>(val).inverse(); }
template<typename T>
inline bool inv(T &val1, T &val2) { val1 = val2.inverse(); return false; }
template<typename T>
auto mag(T&& val) -> decltype(std::forward<T>(val).norm()) { return std::forward<T>(val).norm(); }
template<typename T>
auto dot(T& val1, T& val2) -> decltype(val1.dot(val2)) { return val1.dot(val2); }
template<typename T>
inline T solve(T &val1, T &val2) { return val1.fullPivLu().solve(val2); }

}} // namespace

#endif // SCAFFOLD_MATRIX_EIGEN_H_
