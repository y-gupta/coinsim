#pragma once
#include <cassert>
#include <cmath>
#include <cstdlib>

inline int unique_id(){
  static int next_id = 1;
  return next_id++;
}

inline float frand(float a=0, float b=1){
  float r = float(rand())/RAND_MAX;
  return a + r*(b-a);
}

inline float exp_frand(float beta=1){
  return - log(float(rand())/RAND_MAX)*beta;
}