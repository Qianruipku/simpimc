#ifndef SCAFFOLD_RNG_RNG_H_
#define SCAFFOLD_RNG_RNG_H_

#include <iostream>
#include <random>
#include "../matrix/matrix.h"

namespace scaffold { namespace rand {

class RNG
{
private:
  std::mt19937 rng;  // Make instance of random number generator
protected:
public:
  // Constructor
  RNG(int seed)
   : rng(seed), u_dist(0.,1.)
  {}

  std::uniform_real_distribution<double> u_dist;
  std::normal_distribution<double> normal_dist;

  // Random Functions

  // Generate a random number between 0 and 1
  // return a uniform number in [0,1].
  inline double UnifRand() { return u_dist(rng); }

  // Generate a random number in a real interval.
  // param a one end point of the interval
  // param b the other end of the interval
  // return a inform rand numberin [a,b].
  template <typename T>
  inline double UnifRand(const T a, const T b) { return (b-a)*UnifRand() + a; }

  // Generate a random integer between 1 and a given value.
  // param n the largest value return a uniform random value in [1,...,n]
  inline int UnifRand (const int n) { return floor(UnifRand()*n+1.); }

  // Generate a uniform random vector of length 1
  template <typename T>
  inline void UnifRand(matrix::vec<T>& r)
  {
    for (unsigned int iD=0; iD<r.size(); iD++)
      r(iD) = UnifRand(-1,1);
    r /= matrix::mag(r);
  }

  // Generate a uniform random vector of length l
  template <typename T>
  inline void UnifRand(matrix::vec<T>& r, const T l)
  {
    UnifRand(r);
    r *= l;
  }

  // Generate a normal distribution random number
  inline double NormRand() { return normal_dist(rng); }

  // Generate a normal distribution random number of mean m and variance s
  template <typename T>
  inline double NormRand(const T m, const T s) { return normal_dist(rng)*s + m; }

  // Generate a normal random vector of mean m and variance s
  template <typename T>
  inline void NormRand(matrix::vec<T>& r, const T m, const T s)
  {
    NormRand(r);
    r = r*s + m;
  }

  // Generate a normal random vector of length 1
  template <typename T>
  inline void NormRand(matrix::vec<T>& r)
  {
    for (unsigned int iD=0; iD<r.size(); iD++)
      r(iD) = NormRand();
  }

};

}} // namespace

#endif SCAFFOLD_RNG_RNG_H_
