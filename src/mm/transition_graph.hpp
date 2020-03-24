/**
 * Content
 * Definition of a TransitionGraph, which is a wrapper of trajectory
 * candidates, raw trajectory and UBODT.
 * This class is designed for optimal path inference where
 * Viterbi algorithm is implemented.
 *
 * @author: Can Yang
 * @version: 2020.01.31
 */

#ifndef MM_TRANSITION_GRAPH_HPP
#define MM_TRANSITION_GRAPH_HPP

#include "network/type.hpp"

#include <float.h>

namespace MM
{

struct TGElement {
  const Candidate *c;
  TGElement *prev;
  double ep;
  double cumu_prob;
};

typedef std::vector<TGElement> TGLayer;

class TransitionGraph
{
public:
  /**
   *  Constructor of a TransitionGraph
   *  @param tc: a variational 2D vector
   *  of candidates
   *  @param traj: raw trajectory
   *  @param ubodt: UBODT table
   */
  TransitionGraph(const Traj_Candidates &tc, double gps_error); // TransitionGraph

  static double calc_tp(double sp_dist,double eu_dist);

  static double calc_ep(double dist,double error);

  // Reset the properties of a candidate set
  void reset_layer(TGLayer *layer);

  const TGElement *find_optimal_candidate(const TGLayer &layer);

  OptCandidatePath backtrack();

  std::vector<TGLayer> &get_layers();
private:
  // candidates of a trajectory
  std::vector<TGLayer> layers;
};
}
#endif /* MM_TRANSITION_GRAPH_HPP */
