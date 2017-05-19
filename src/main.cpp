//author: yash gupta (2013CS10302)

#include "sim.h"
#include "bitcoin.h"
#include "events.h"
#include <iostream>
using namespace std;



int main(int arc, char **argv){
  cout<<"BITCOIN NETWORK SIMULATOR"<<endl;
  Simulator S;
  S.schedule(0, new Genesis());
  S.run();
  return 0;
}