#ifndef SCAFFOLD_COMMUNICATION_MPI_DATATYPE_H_
#define SCAFFOLD_COMMUNICATION_MPI_DATATYPE_H_

#if USE_MPI
  #include <mpi.h>
#endif
#if USE_OPENMP
  #include <omp.h>
#endif
#include "../matrix/matrix.h"

namespace scaffold { namespace parallel {

#if USE_MPI
// Template to retrieve traits of any MPI object
template <class T>
struct MPITypeTraits {
  static inline MPI_Datatype GetType(T& val);
  static inline size_t GetSize(T& val);
  static inline void* GetAddr(T& val);
};

// Specialization of MPITypeTraits for primitive types
#define PRIMITIVE(Type, MpiType) \
        template<> \
        inline MPI_Datatype MPITypeTraits<Type>::GetType(Type&) { return MpiType; } \
        template<> \
        inline size_t MPITypeTraits<Type>::GetSize(Type&) { return 1; } \
        template<> \
        inline void* MPITypeTraits<Type>::GetAddr(Type& val) { return &val; }
  PRIMITIVE(char, MPI::CHAR);
  PRIMITIVE(wchar_t, MPI::WCHAR);
  PRIMITIVE(short, MPI::SHORT);
  PRIMITIVE(int, MPI::INT);
  PRIMITIVE(long, MPI::LONG);
  PRIMITIVE(signed char, MPI::SIGNED_CHAR);
  PRIMITIVE(unsigned char, MPI::UNSIGNED_CHAR);
  PRIMITIVE(unsigned short, MPI::UNSIGNED_SHORT);
  PRIMITIVE(unsigned int, MPI::UNSIGNED);
  PRIMITIVE(unsigned long, MPI::UNSIGNED_LONG);
  PRIMITIVE(unsigned long long, MPI::UNSIGNED_LONG_LONG);
  PRIMITIVE(bool, MPI::BOOL);
  PRIMITIVE(float, MPI::FLOAT);
  PRIMITIVE(double, MPI::DOUBLE);
  PRIMITIVE(std::complex<float>, MPI::COMPLEX);
  PRIMITIVE(std::complex<double>, MPI::DOUBLE_COMPLEX);
#undef PRIMITIVE

// Specialization of MPITypeTraits for armadillo types
#ifdef USE_ARMADILLO
  #define ARMATYPE(Type, ElemType, MpiType) \
          template<> \
          inline MPI_Datatype MPITypeTraits<Type>::GetType(Type&) { return MpiType; } \
          template<> \
          inline size_t MPITypeTraits<Type>::GetSize(Type& val) { return val.size(); } \
          template<> \
          inline void* MPITypeTraits<Type>::GetAddr(Type& val) { return val.memptr(); }
    ARMATYPE(matrix::mat<int>, int, MPI::INT);
    ARMATYPE(matrix::vec<int>, int, MPI::INT);
    ARMATYPE(matrix::mat<double>, double, MPI::DOUBLE);
    ARMATYPE(matrix::vec<double>, double, MPI::DOUBLE);
    ARMATYPE(matrix::mat<std::complex<double> >, std::complex<double>, MPI::DOUBLE_COMPLEX);
    ARMATYPE(matrix::vec<std::complex<double> >, std::complex<double>, MPI::DOUBLE_COMPLEX);
    ARMATYPE(matrix::mat<float>, float, MPI::FLOAT);
    ARMATYPE(matrix::vec<float>, float, MPI::FLOAT);
    ARMATYPE(matrix::mat<std::complex<float> >, std::complex<float>, MPI::COMPLEX);
    ARMATYPE(matrix::vec<std::complex<float> >, std::complex<float>, MPI::COMPLEX);
  #undef ARMATYPE
#endif

// Specialization of MPITypeTraits for eigen types
#ifdef USE_EIGEN
  #define EIGENTYPE(Type, ElemType, MpiType) \
          template<> \
          inline MPI_Datatype MPITypeTraits<Type>::GetType(Type&) { return MpiType; } \
          template<> \
          inline size_t MPITypeTraits<Type>::GetSize(Type& val) { return val.size(); } \
          template<> \
          inline void* MPITypeTraits<Type>::GetAddr(Type& val) { return val.data(); }
    EIGENTYPE(matrix::mat<int>, int, MPI::INT);
    EIGENTYPE(matrix::vec<int>, int, MPI::INT);
    EIGENTYPE(matrix::mat<double>, double, MPI::DOUBLE);
    EIGENTYPE(matrix::vec<double>, double, MPI::DOUBLE);
    EIGENTYPE(matrix::mat<std::complex<double> >, std::complex<double>, MPI::DOUBLE_COMPLEX);
    EIGENTYPE(matrix::vec<std::complex<double> >, std::complex<double>, MPI::DOUBLE_COMPLEX);
    EIGENTYPE(matrix::mat<float>, float, MPI::FLOAT);
    EIGENTYPE(matrix::vec<float>, float, MPI::FLOAT);
    EIGENTYPE(matrix::mat<std::complex<float> >, std::complex<float>, MPI::COMPLEX);
    EIGENTYPE(matrix::vec<std::complex<float> >, std::complex<float>, MPI::COMPLEX);
  #undef EIGENTYPE
#endif

#endif

}} // namespace

#endif // SCAFFOLD_COMMUNICATION_MPI_DATATYPE_H_
