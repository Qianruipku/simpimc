#ifndef SCAFFOLD_IO_HDF5_DATATYPE_H_
#define SCAFFOLD_IO_HDF5_DATATYPE_H_

#include "H5Cpp.h"
#include "../matrix/matrix.h"

namespace scaffold { namespace io {

// Template to retrieve traits of any HDF5 object
template <class T>
struct HDF5TypeTraits {
  static H5::PredType GetType(T& val);
  static size_t GetSize(T& val);
  static void* GetAddr(T& val);
  static const int GetDim(T& val, int d);
  static const int GetRank(T& val);
};

  // Specialization of HDF5TypeTraits for primitive types
#define PRIMITIVE(Type, H5PredType, H5Type) \
        template<> \
        H5::PredType HDF5TypeTraits<Type>::GetType(Type&) { return H5Type; } \
        template<> \
        size_t HDF5TypeTraits<Type>::GetSize(Type&) { return 1; } \
        template<> \
        void* HDF5TypeTraits<Type>::GetAddr(Type& val) { return &val; } \
        template<> \
        const int HDF5TypeTraits<Type>::GetDim(Type& val, int d) { return 1; } \
        template<> \
        const int HDF5TypeTraits<Type>::GetRank(Type& val) { return 0; }
  PRIMITIVE(char, H5::IntType, H5::PredType::NATIVE_CHAR);
  PRIMITIVE(std::string, H5::StrType, H5::PredType::NATIVE_CHAR);
  PRIMITIVE(short, H5::IntType, H5::PredType::NATIVE_SHORT);
  PRIMITIVE(int, H5::IntType, H5::PredType::NATIVE_INT);
  PRIMITIVE(long, H5::IntType, H5::PredType::NATIVE_LONG);
  PRIMITIVE(signed char, H5::IntType, H5::PredType::NATIVE_SCHAR);
  PRIMITIVE(unsigned char, H5::IntType, H5::PredType::NATIVE_UCHAR);
  PRIMITIVE(unsigned short, H5::IntType, H5::PredType::NATIVE_USHORT);
  PRIMITIVE(unsigned int, H5::IntType, H5::PredType::NATIVE_UINT);
  PRIMITIVE(unsigned long, H5::IntType, H5::PredType::NATIVE_ULONG);
  PRIMITIVE(unsigned long long, H5::IntType, H5::PredType::NATIVE_ULLONG);
  PRIMITIVE(std::complex<double>, H5::FloatType, H5::PredType::NATIVE_DOUBLE);
  PRIMITIVE(std::complex<float>, H5::FloatType, H5::PredType::NATIVE_FLOAT);
  PRIMITIVE(double, H5::FloatType, H5::PredType::NATIVE_DOUBLE);
  PRIMITIVE(float, H5::FloatType, H5::PredType::NATIVE_FLOAT);
#undef PRIMITIVE

// Specialization of HDF5TypeTraits for armadillo types
#ifdef USE_ARMADILLO
  #define ARMATYPE(Type, ElemType, H5PredType, H5Type) \
          template<> \
          H5::PredType HDF5TypeTraits<Type>::GetType(Type&) { return H5Type; } \
          template<> \
          size_t HDF5TypeTraits<Type>::GetSize(Type& val) { return val.size(); } \
          template<> \
          void* HDF5TypeTraits<Type>::GetAddr(Type& val) { return val.memptr(); } \
          template<> \
          const int HDF5TypeTraits<Type>::GetDim(Type& val, int d) { if (d == 0) { return val.n_rows; } else { return val.n_cols; } } \
          template<> \
          const int HDF5TypeTraits<Type>::GetRank(Type& val) { return 1; }
    ARMATYPE(matrix::vec<int>, int, H5::IntType, H5::PredType::NATIVE_INT);
    ARMATYPE(matrix::vec<unsigned int>, unsigned int, H5::IntType, H5::PredType::NATIVE_UINT);
    ARMATYPE(matrix::vec<float>, float, H5::FloatType, H5::PredType::NATIVE_FLOAT);
    ARMATYPE(matrix::vec<double>, double, H5::FloatType, H5::PredType::NATIVE_DOUBLE);
  #undef ARMATYPE

  #define ARMATYPE(Type, ElemType, H5PredType, H5Type) \
          template<> \
          H5::PredType HDF5TypeTraits<Type>::GetType(Type&) { return H5Type; } \
          template<> \
          size_t HDF5TypeTraits<Type>::GetSize(Type& val) { return val.size(); } \
          template<> \
          void* HDF5TypeTraits<Type>::GetAddr(Type& val) { return val.memptr(); } \
          template<> \
          const int HDF5TypeTraits<Type>::GetDim(Type& val, int d) { if (d == 0) { return val.n_cols; } else { return val.n_rows; } } \
          template<> \
          const int HDF5TypeTraits<Type>::GetRank(Type& val) { return 2; }
    ARMATYPE(matrix::mat<int>, int, H5::IntType, H5::PredType::NATIVE_INT);
    ARMATYPE(matrix::mat<unsigned int>, unsigned int, H5::IntType, H5::PredType::NATIVE_UINT);
    ARMATYPE(matrix::mat<float>, float, H5::FloatType, H5::PredType::NATIVE_FLOAT);
    ARMATYPE(matrix::mat<double>, double, H5::FloatType, H5::PredType::NATIVE_DOUBLE);
  #undef ARMATYPE

  #define ARMATYPE(Type, ElemType, H5PredType, H5Type) \
          template<> \
          H5::PredType HDF5TypeTraits<Type>::GetType(Type&) { return H5Type; } \
          template<> \
          size_t HDF5TypeTraits<Type>::GetSize(Type& val) { return val.size(); } \
          template<> \
          void* HDF5TypeTraits<Type>::GetAddr(Type& val) { return val.memptr(); } \
          template<> \
          const int HDF5TypeTraits<Type>::GetDim(Type& val, int d) { if (d == 0) { return val.n_slices; } else if (d == 1) { return val.n_cols; } else { return val.n_rows; } } \
          template<> \
          const int HDF5TypeTraits<Type>::GetRank(Type& val) { return 3; }
    ARMATYPE(matrix::cube<int>, int, H5::IntType, H5::PredType::NATIVE_INT);
    ARMATYPE(matrix::cube<unsigned int>, unsigned int, H5::IntType, H5::PredType::NATIVE_UINT);
    ARMATYPE(matrix::cube<float>, float, H5::FloatType, H5::PredType::NATIVE_FLOAT);
    ARMATYPE(matrix::cube<double>, double, H5::FloatType, H5::PredType::NATIVE_DOUBLE);
  #undef ARMATYPE
#endif

// Specialization of HDF5TypeTraits for eigen types
#ifdef USE_EIGEN
  #define EIGENTYPE(Type, ElemType, H5PredType, H5Type) \
          template<> \
          H5::PredType HDF5TypeTraits<Type>::GetType(Type&) { return H5Type; } \
          template<> \
          size_t HDF5TypeTraits<Type>::GetSize(Type& val) { return val.size(); } \
          template<> \
          void* HDF5TypeTraits<Type>::GetAddr(Type& val) { return val.data(); } \
          template<> \
          const int HDF5TypeTraits<Type>::GetDim(Type& val, int d) { if (d == 0) { return val.rows(); } else { return val.cols(); } } \
          template<> \
          const int HDF5TypeTraits<Type>::GetRank(Type& val) { return 1; }
    EIGENTYPE(matrix::vec<int>, int, H5::IntType, H5::PredType::NATIVE_INT);
    EIGENTYPE(matrix::vec<unsigned int>, unsigned int, H5::IntType, H5::PredType::NATIVE_UINT);
    EIGENTYPE(matrix::vec<float>, float, H5::FloatType, H5::PredType::NATIVE_FLOAT);
    EIGENTYPE(matrix::vec<double>, double, H5::FloatType, H5::PredType::NATIVE_DOUBLE);
  #undef EIGENTYPE

  #define EIGENTYPE(Type, ElemType, H5PredType, H5Type) \
          template<> \
          H5::PredType HDF5TypeTraits<Type>::GetType(Type&) { return H5Type; } \
          template<> \
          size_t HDF5TypeTraits<Type>::GetSize(Type& val) { return val.size(); } \
          template<> \
          void* HDF5TypeTraits<Type>::GetAddr(Type& val) { return val.data(); } \
          template<> \
          const int HDF5TypeTraits<Type>::GetDim(Type& val, int d) { if (d == 0) { return val.cols(); } else { return val.rows(); } } \
          template<> \
          const int HDF5TypeTraits<Type>::GetRank(Type& val) { return 2; }
    EIGENTYPE(matrix::mat<int>, int, H5::IntType, H5::PredType::NATIVE_INT);
    EIGENTYPE(matrix::mat<unsigned int>, unsigned int, H5::IntType, H5::PredType::NATIVE_UINT);
    EIGENTYPE(matrix::mat<float>, float, H5::FloatType, H5::PredType::NATIVE_FLOAT);
    EIGENTYPE(matrix::mat<double>, double, H5::FloatType, H5::PredType::NATIVE_DOUBLE);
  #undef EIGENTYPE

  #define EIGENTYPE(Type, ElemType, H5PredType, H5Type) \
          template<> \
          H5::PredType HDF5TypeTraits<Type>::GetType(Type&) { return H5Type; } \
          template<> \
          size_t HDF5TypeTraits<Type>::GetSize(Type& val) { return val.data.size(); } \
          template<> \
          void* HDF5TypeTraits<Type>::GetAddr(Type& val) { return val.data.data(); } \
          template<> \
          const int HDF5TypeTraits<Type>::GetDim(Type& val, int d) { if (d == 0) { return val.n_rows; } else if (d == 1) { return val.n_cols; } else { return val.n_slices; } } \
          template<> \
          const int HDF5TypeTraits<Type>::GetRank(Type& val) { return 3; }
    EIGENTYPE(matrix::cube<int>, int, H5::IntType, H5::PredType::NATIVE_INT);
    EIGENTYPE(matrix::cube<unsigned int>, unsigned int, H5::IntType, H5::PredType::NATIVE_UINT);
    EIGENTYPE(matrix::cube<float>, float, H5::FloatType, H5::PredType::NATIVE_FLOAT);
    EIGENTYPE(matrix::cube<double>, double, H5::FloatType, H5::PredType::NATIVE_DOUBLE);
  #undef EIGENTYPE
#endif

}} // namespace

#endif // SCAFFOLD_IO_HDF5_DATATYPE_H_
