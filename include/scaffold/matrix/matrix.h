#ifndef SCAFFOLD_MATRIX_MATRIX_H_
#define SCAFFOLD_MATRIX_MATRIX_H_

#ifdef USE_EIGEN
  #define EIGEN_MATRIXBASE_PLUGIN <Matrix/eigen_matrix_additions.h>
  #include "eigen.h"
#else
  #include "armadillo.h"
#endif

namespace scaffold { namespace matrix {

// FIXME: Make this work
//// Calculates determinant of A and its inverse (in place) by gauss-jordan elimination with full pivoting
//// Adapted from Numerical Recipes in C
//inline double GJDetInverse (mat<double> &A)
//{
//  int n = A.n_rows;
//  const int max_size = 2000;
//  int col_index[max_size], row_index[max_size], ipiv[max_size];
//  double big, dum, piv_inv, temp;
//  int icol, irow;
//
//  for (int j=0; j<n; j++)
//    ipiv[j] = -1;
//
//  double det_A = 1.;
//  for (int i=0; i<n; i++) {
//    big = 0.0;
//    for (int j=0; j<n; j++)
//      if (ipiv[j] != 0)
//        for (int k=0; k<n; k++) {
//          if (ipiv[k] == -1) {
//            if (fabs(A(j,k)) >= big) {
//              big = fabs(A(j,k));
//              irow = j;
//              icol = k;
//            }
//          }
//          else if (ipiv[k] > 0) {
//            std::cerr << "ERROR: GJInverse: Singular matrix!" << std::endl;
//            return 0;
//          }
//        }
//    ++(ipiv[icol]);
//
//    if (irow != icol)
//      for (int l=0; l<n; l++)
//        std::swap(A(irow,l), A(icol,l));
//
//    row_index[i] = irow;
//    col_index[i] = icol;
//    if (A(icol,icol) == 0.) {
//      std::cerr << "ERROR: GJInverse: Singular matrix!" << std::endl;
//      return 0;
//    }
//    det_A *= A(icol,icol);
//    piv_inv = 1./A(icol,icol);
//    A(icol,icol) = 1.;
//    for (int l=0; l<n; l++)
//      A(icol,l) *= piv_inv;
//    for (int ll=0; ll<n; ll++)
//      if (ll != icol) {
//        double dum = A(ll,icol);
//        A(ll,icol) = 0.;
//        for (int l=0; l<n; l++)
//          A(ll,l) -= A(icol,l)*dum;
//      }
//  }
//  // Now unscramble the permutations
//  for (int l=n-1; l>=0; l--) {
//    if (row_index[l] != col_index[l]) {
//      for (int k=0; k<n ; k++)
//        std::swap(A(k,row_index[l]), A(k,col_index[l]));
//      det_A *= -1;
//    }
//  }
//  return det_A;
//}

}} // namespace

#endif // SCAFFOLD_MATRIX_MATRIX_H_
