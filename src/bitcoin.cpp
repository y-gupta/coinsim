#include "bitcoin.h"

int Peer::next_id = 0;
float Peer::fraction_slow = 0.25;
float Peer::beta_trans_time = 1;
float Peer::beta_beta_T = 10;