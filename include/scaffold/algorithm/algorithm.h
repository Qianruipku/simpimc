#ifndef SCAFFOLD_ALGORITHM_H_
#define SCAFFOLD_ALGORITHM_H_

#include <vector>
#include <algorithm>
#include <map>

namespace scaffold { namespace algorithm {

  /// Compare the values of two vectors
  template<class T>
  struct CompareVec
  {
    bool operator() (const std::vector<T> &a, const std::vector<T> &b) {
      for (uint32_t i=0; i<a.size(); i++)
        if (a[i] != b[i])
          return (a[i] > b[i]);
      return (a[0] > b[0]);
    }
  };

  inline double CMag2 (const std::complex<double> &z1, const std::complex<double> &z2)
  {
    return (z1.real()*z2.real() + z1.imag()*z2.imag());
  }

  inline double CMag (const std::complex<double> &z1, const std::complex<double> &z2)
  {
    return sqrt(CMag2(z1,z2));
  }

  inline unsigned int factorial(const unsigned int x)
  {
    return (x == 1 ? x : x * factorial(x - 1));
  }

  inline unsigned int n_choose_k(const unsigned int n, const unsigned int k)
  {
    unsigned int n_factorial_over_n_minus_k_factorial = 1;
    for (unsigned int i=n; i>n-k; --i)
      n_factorial_over_n_minus_k_factorial *= i;
    return n_factorial_over_n_minus_k_factorial/factorial(k);
  }

  template <typename T>
  inline bool fequal(const T a, const T b, const T tol)
  {
    return (fabs(a-b) < tol);
  }

  template <typename Iterator>
  inline bool next_combination(const Iterator first, Iterator k, const Iterator last)
  {
     /* Credits: Thomas Draper */
     if ((first == last) || (first == k) || (last == k))
        return false;
     Iterator itr1 = first;
     Iterator itr2 = last;
     ++itr1;
     if (last == itr1)
        return false;
     itr1 = last;
     --itr1;
     itr1 = k;
     --itr2;
     while (first != itr1)
     {
        if (*--itr1 < *itr2)
        {
           Iterator j = k;
           while (!(*itr1 < *j)) ++j;
           std::iter_swap(itr1,j);
           ++itr1;
           ++j;
           itr2 = k;
           std::rotate(itr1,j,last);
           while (last != j)
           {
              ++j;
              ++itr2;
           }
           std::rotate(k,itr2,last);
           return true;
        }
     }
     std::rotate(first,k,last);
     return false;
  }

  template <typename T>
  void GenPerm(std::vector<std::vector<T>> &vs, std::vector<T> vals)
  {
    std::sort(vals.begin(), vals.end());
    do {
      vs.push_back(vals);
    } while(std::next_permutation(vals.begin(),vals.end()));

  }

  template <typename T>
  void GenCombPermK(std::vector<std::vector<T>> &vs, std::vector<T> vals, unsigned int k, bool duplicates=false, bool permutations=false)
  {
    if (duplicates) {
      unsigned int n = vals.size();
      for (unsigned int i=0; i<k-1; i++)
        for (unsigned int j=0; j<n; j++)
          vals.push_back(vals[j]);
    }
    std::sort(vals.begin(), vals.end());

    do {
      std::vector<T> v;
      for (unsigned int i=0; i<k; ++i)
        v.push_back(vals[i]);
      std::sort(v.begin(), v.end());
      do {
        vs.push_back(v);
      } while(std::next_permutation(v.begin(),v.end()) && permutations);
    } while(next_combination(vals.begin(),vals.begin() + k,vals.end()));

  }

}}

#endif
