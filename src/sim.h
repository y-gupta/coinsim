#pragma once
#include <map>
#include <cstring>
#include <cassert>
#include <iostream>
#include <unistd.h>
#include <omp.h>
using namespace std;

class Simulator;
class Peer;
class Event{
public:
  char name[100];
  bool deleted;
  Peer *peer;
  Event(){
    deleted = false;
    peer = NULL;
    strcpy(name, "default");
  }
  virtual void execute(float t, Simulator *s){
  }
};
class Simulator{
public:
  float t;
  int state;
  multimap<float, Event*> queue;
  Simulator(){
    t = 0;
  }
  void schedule(float delta, Event *e){
    assert(delta >= 0);
    queue.emplace(t + delta, e);
  }
  void run(){
    state = 0;
    #pragma omp parallel num_threads(1)
    {
      if(omp_get_thread_num() == 0){
        while(!queue.empty()){

          #pragma omp flush(state)
          if(state == 1){
            cout<<"\nPAUSED\n";
            while(state == 1){
              usleep(10000); //sleep for 10 ms
              #pragma omp flush(state)
            }
            if(state == 0)
              cout<<"RESUMED\n";
          }
          if(state == 2){
            cout<<"\nQUITTING\n";
            break;
          }

          auto t_e = queue.begin();
          float delta = t_e->first - t;
          usleep(delta*1000000*0.0);
          t = t_e->first;
          auto event = t_e->second;
          queue.erase(t_e);
          if(event->deleted == false){
            //cout<<"Executing "<<event->name<<" at "<<t<<endl;
            event->execute(t, this);
          }
          delete event;
        }
      }else{
        while(1){
          int ch = cin.get();
          if(ch == 'q')
            state = 2;
          else if(ch == 'p')
            state = 1- state;
          #pragma omp flush(state)
          if(state == 2)
            break;
        }
      }
    }
  }
};