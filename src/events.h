#pragma once
#include "common.h"
#include "sim.h"
#include "bitcoin.h"

#include <cstdio>

class Genesis: public Event{
public:
  Genesis(){
    sprintf(name, "Genesis");
  }
  void execute(float t, Simulator *S) override;
};

class RecvBlock: public Event{
public:
  Block *blk;
  int from;
  RecvBlock(Peer *_peer, Block *_blk, int _from){
    peer = _peer;
    blk = new Block(*_blk);
    // *blk = *_blk;
    from = _from;
    sprintf(name, "%d:RecvBlock(%d)", peer->id, blk->id);
  }
  void execute(float t, Simulator *S) override;
};

class RecvTransaction: public Event{
public:
  Transaction tnx;
  int from;
  RecvTransaction(Peer *_peer,const Transaction &_tnx, int _from){
    peer = _peer;
    tnx = _tnx;
    from = _from;
    sprintf(name, "%d:RecvTransaction(%d)", peer->id, tnx.id);
  }
  void execute(float t, Simulator *S) override;
};

class MakeTransaction: public Event{
public:
  MakeTransaction(Peer *_peer){
    peer = _peer;
    sprintf(name, "%d:MakeTransaction", peer->id);
  }
  void execute(float t, Simulator *S) override;
};

class MakeBlock: public Event{
public:
  MakeBlock(Peer *_peer){
    peer = _peer;
    sprintf(name, "%d:MakeBlock", peer->id);
  }
  void execute(float t, Simulator *S) override;
};

class ShowBalance: public Event{
public:
  ShowBalance(){
    strcpy(name, "ShowBalance");
  }
  void execute(float t, Simulator *S) override;
};

class ExitSim: public Event{
public:
  void execute(float t, Simulator *S) override{
    S->state = 2;
  }
};
