#pragma once
#include "common.h"

#include <vector>
#include <set>
#include <map>
#include <cstdio>
#include <iostream>

using namespace std;

class Transaction{
public:
  int id, from_id, to_id, amt;
  Transaction(){
    id = from_id = to_id = amt = -1;
  }
  Transaction(int from,int to,int _amt){
    id = unique_id();
    from_id = from;
    to_id = to;
    amt = _amt;
  }
  void print(){
    if(from_id == -1)
      cout<<"TNX#"<<id<<": (COINBASE -> peer#"<<to_id<<") "<<amt<<" coins\n";
    else
      cout<<"TNX#"<<id<<": (peer#"<<from_id<<" -> peer#"<<to_id<<") "<<amt<<" coins\n";
  }
};

class Block{
public:
  int id, parent_id, depth;
  float recv_time;
  bool rooted;
  vector<Transaction> tnxs;
  Block(){
    id = unique_id();
    rooted = true;
    parent_id = -1;
    recv_time = 0;
    depth = 0;
  }
};

class Event;
class Peer{
public:
  int id;
  bool is_slow;
  float beta_T;
  int balance;

  map<int, Transaction> tnxs;

  Block *deepest_block;
  map<int, Block*> blocks;
  multimap<int, Block*> orphan_blocks; // map of missing_blk => orphan_blk
  Event *make_block_event;

  vector<int> nbrs;

  static int next_id;
  static float fraction_slow;
  static float beta_trans_time; //mean inter-transaction time
  static float beta_beta_T; //mean of beta_T's assigned to different nodes.

  Peer(){
    if(frand() > fraction_slow)
      is_slow = false;
    else
      is_slow = true;
    id = next_id;
    next_id++;
    deepest_block = NULL;
    balance = 0;
    beta_T = exp_frand(beta_beta_T);
  }

  ~Peer(){
    char fname[100];
    sprintf(fname, "out/peer%03d.gv",id);
    FILE *ft = fopen(fname, "wt");
    fprintf(ft, "digraph blockchain {\n");
    fprintf(ft, "  graph [label=\"peer#%d-%s-%0.2f\"];\n", id, is_slow?"slow":"fast", beta_T);
    fprintf(ft, "  node [shape=box, style=filled];\n");
    fprintf(ft, "  rankdir=\"LR\"; size=\"20,5\";\n");
    for(auto &it: blocks){
      fprintf(ft, "  %d -> %d;\n",it.second->parent_id, it.second->id);
      if(it.second->tnxs[0].to_id == id)
        fprintf(ft, "  %d [color=blue];\n", it.second->id);
    }
    fprintf(ft, "\n}\n\n");
    fclose(ft);
  }

  static float get_prop(int i,int j){
    static float *prop = NULL;
    if(prop == NULL){
      prop = new float[next_id*next_id];
      for(int i=0;i<next_id;i++){
        for(int j=i+1;j<next_id;j++){
          float p = frand(0.010,0.500);
          prop[j*next_id + i] = prop[i*next_id + j] = p;
        }
      }
    }
    assert(i < next_id && j < next_id);
    return prop[i*next_id + j];
  }
  float get_latency(Peer *b, bool has_block=false){
    float c = 100.0/8; //100Mbps
    if(is_slow || b->is_slow)
      c = 5.0/8;
    float L = get_prop(id,b->id) + (has_block?1:0)/c + exp_frand(12/(1024*c));
    return L;
  }

  void update_balance(){
    if(deepest_block == NULL)
      return;
    balance = 0;
    Block *blk = deepest_block;
    while(1){
      for(auto &tnx: blk->tnxs){
        if(tnx.to_id == id)
          balance += tnx.amt;
        if(tnx.from_id == id)
          balance -= tnx.amt;
      }
      if(blk->parent_id == blk->id)
        break;
      blk = blocks.at(blk->parent_id);
    }
    if(balance < 0)
    {
      0;
    }
  }

  void flush_transactions(Block *new_blk){
    if(deepest_block == NULL)
      return;
    Block *blk = deepest_block;
    auto new_tnxs = tnxs;
    while(blk->parent_id != blk->id){
      for(auto &tnx: blk->tnxs){
        new_tnxs.erase(tnx.id);
      }
      blk = blocks.at(blk->parent_id);
    }
    for(auto &it: new_tnxs){
      new_blk->tnxs.push_back(it.second);
    }
  }

  bool update_deepest(Block *blk){
    if(!blk->rooted)
      return false;
    bool updated = false;
    if( deepest_block == NULL ||
        blk->depth > deepest_block->depth ||
        (blk->depth == deepest_block->depth && blk->recv_time < deepest_block->recv_time)){
      deepest_block = blk;
      updated = true;
    }

    auto orphans = orphan_blocks.equal_range(blk->id);
    for(auto &it=orphans.first; it != orphans.second;){
      auto child_blk = it->second;
      child_blk->rooted = true;
      orphan_blocks.erase(it++);
      updated |= update_deepest(child_blk);
    }
    return updated;
  }
};

