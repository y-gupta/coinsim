#include "events.h"

#include <vector>

using namespace std;
vector<Peer> peers;
int N;

void Genesis::execute(float t, Simulator *S){
  N = 10;
  int max_nbrs = 4;

  if(max_nbrs >= N-1)
    max_nbrs = N-1;

  Peer::fraction_slow = 0.5;
  Peer::beta_trans_time = 1*N;
  Peer::beta_beta_T = 20*N;

  S->schedule(20*Peer::beta_beta_T/N, new ExitSim());

  peers.resize(N);

  Block genesis_block;
  genesis_block.id = unique_id();
  genesis_block.parent_id = genesis_block.id;
  genesis_block.tnxs.push_back(Transaction(-1,0,50));

  for(int j=0; j < peers.size(); j++){
    auto &peer = peers[j];
    int nbrs_needed = (rand() % max_nbrs) + 1;
    for(int i=peers.size()-1; i >= 0; i--){
      if(i == j)
        continue;
      if(frand()*i <= nbrs_needed){
        peer.nbrs.push_back(i);
        nbrs_needed--;
      }
    }

    S->schedule(0, new RecvBlock(&peer, &genesis_block, -1));

    peer.make_block_event = new MakeBlock(&peer);
    S->schedule(exp_frand(peer.beta_T), peer.make_block_event);
    S->schedule(exp_frand(peer.beta_trans_time), new MakeTransaction(&peer));
  }
  S->schedule(0, new ShowBalance());
}

void RecvBlock::execute(float t, Simulator *S){
  if(peer->blocks.find(blk->id) != peer->blocks.end()) //already received this block from someone else!
    return;
  peer->blocks.emplace(blk->id, blk);
  blk->recv_time = t;

  auto parent_it = peer->blocks.find(blk->parent_id);
  if(parent_it == peer->blocks.end()){
    peer->orphan_blocks.emplace(blk->parent_id, blk);
    blk->rooted = false;
  }else{
    if(!parent_it->second->rooted)
      blk->rooted = false;
    else
      blk->rooted = true;
  }


  if(peer->update_deepest(blk)){
    peer->update_balance();
    //received a new longest-chain block. reset next MakeBlock event
    peer->make_block_event->deleted = true;
    peer->make_block_event = new MakeBlock(peer);
    S->schedule(exp_frand(peer->beta_T), peer->make_block_event);
  }

  if(from == -1) //genesis should not be broadcasted
    return;
  for(auto j: peer->nbrs){ //broadcast the block!
    if(j==from)
      continue;
    auto nbr = &peers[j];
    S->schedule(peer->get_latency(nbr, true), new RecvBlock(nbr, blk, peer->id));
  }
}

void MakeTransaction::execute(float t, Simulator *S){
  if(peer->balance > 0){
    // cout<<"peer#"<<peer->id<<"'s balance: "<<peer->balance<<endl;
    int amt = 1 + rand() % (peer->balance>100?100:peer->balance);
    Transaction tnx(peer->id, rand() % N, amt);
    peer->balance -= tnx.amt;
    cout<<" (broadcast) ";
    tnx.print();
    S->schedule(0, new RecvTransaction(peer, tnx, -1));
  }
  S->schedule(exp_frand(peer->beta_trans_time), new MakeTransaction(peer));
}

void RecvTransaction::execute(float t, Simulator *S){
  if(peer->tnxs.find(tnx.id) != peer->tnxs.end()) //already received this tnx from someone else!
    return;
  peer->tnxs.emplace(tnx.id, tnx);

  for(auto j: peer->nbrs){ //broadcast the tnx!
    if(j==from)
      continue;
    auto nbr = &peers[j];
    S->schedule(peer->get_latency(nbr), new RecvTransaction(nbr, tnx, peer->id));
  }
}

void MakeBlock::execute(float t, Simulator *S){
  Block *blk = new Block();
  blk->parent_id = peer->deepest_block->id;
  blk->depth = peer->deepest_block->depth + 1;
  blk->recv_time = t;
  blk->rooted = true;
  blk->tnxs.push_back(Transaction(-1,peer->id,50));
  peer->flush_transactions(blk);
  peer->deepest_block = blk;
  peer->blocks.emplace(blk->id, blk);
  peer->update_balance();
  cout<<"====>>>>\n";
  cout<<"Peer#" <<peer->id<<" Made blk#"<<blk->id<<" with "<<blk->tnxs.size()<<" tnxs and depth "<<blk->depth<<"\n";

  Block *tmp_blk = blk;

  for(int i=0;i<10;i++){
    tmp_blk = peer->blocks.at(tmp_blk->parent_id);
    cout<<" -> blk#"<<tmp_blk->id;
    if(tmp_blk->parent_id == tmp_blk->id)
      break;
  }
  cout<<endl;

  // for(auto &tnx: blk->tnxs){
  //   tnx.print();
  // }
  cout<<"<<<<====\n\n";
  for(auto j: peer->nbrs){ //broadcast the block!
    auto nbr = &peers[j];
    S->schedule(peer->get_latency(nbr, true), new RecvBlock(nbr, blk, peer->id));
  }

  peer->make_block_event = new MakeBlock(peer);
  S->schedule(exp_frand(peer->beta_T), peer->make_block_event);
}

void ShowBalance::execute(float t, Simulator *S){
  for(auto &peer: peers){
    cout<<"peer#"<<peer.id<<":"<<peer.balance<<" (blk#"<<peer.deepest_block->id<<"); ";
  }
  cout<<endl;
  S->schedule(10, new ShowBalance());
}