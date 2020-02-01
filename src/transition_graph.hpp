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

#include <float.h>

#include "types.hpp"
#include "network.hpp"
#include "ubodt.hpp"
#include "debug.h"

namespace MM
{
class TransitionGraph
{
public:
  /**
   *  Constructor of a TransitionGraph
   *  @param traj_candidates: a variational 2D vector
   *  of candidates
   *  @param traj: raw trajectory
   *  @param ubodt: UBODT table
   */
  TransitionGraph(Traj_Candidates *traj_candidates, const LineString &traj,
                  UBODT &ubodt, double delta = 5000) :
    m_traj_candidates(traj_candidates),
    m_traj(traj),
    m_ubodt(ubodt),
    eu_distances(cal_eu_dist(traj)),
    DISTANCE_NOT_FOUND(delta){
  };
  /**
   * Viterbi algorithm, infer an optimal path in the transition
   * graph
   *
   * @param  pf, penalty factor
   * @return  O_Path, a optimal path containing candidates
   * matched for each point in a trajectory. In case that no
   * path is found, an empty path is returned.
   */
  O_Path viterbi(double pf=0)
  {
    O_Path opath;
    if (m_traj_candidates->empty()) return opath;
    int N = m_traj_candidates->size();
    /* Update transition probabilities */
    Traj_Candidates::iterator csa = m_traj_candidates->begin();
    /* Initialize the cumu probabilities of the first layer */
    Point_Candidates::iterator ca = csa->begin();
    while (ca != csa->end())
    {
      ca->cumu_prob = ca->obs_prob;
      ++ca;
    }
    /* Updating the cumu probabilities of subsequent layers */
    Traj_Candidates::iterator csb = m_traj_candidates->begin();
    ++csb;
    while (csb != m_traj_candidates->end())
    {
      Point_Candidates::iterator ca = csa->begin();
      double eu_dist=eu_distances[
        std::distance(m_traj_candidates->begin(),csa)];
      while (ca != csa->end())
      {
        Point_Candidates::iterator cb = csb->begin();
        while (cb != csb->end())
        {
          // Calculate transition probability
          double sp_dist = get_sp_dist_penalized(ca,cb,pf);
          // A degenerate case is that the same point
          // is reported multiple times where both eu_dist and sp_dist = 0
          double tran_prob = 1.0;
          if (eu_dist<0.00001) {
            tran_prob =sp_dist>0.00001 ? 0 : 1.0;
          } else {
            tran_prob =eu_dist>sp_dist ? sp_dist/eu_dist : eu_dist/sp_dist;
          }

          if (ca->cumu_prob + tran_prob * cb->obs_prob >= cb->cumu_prob)
          {
            cb->cumu_prob = ca->cumu_prob + tran_prob * cb->obs_prob;
            cb->prev = &(*ca);
            cb->sp_dist = sp_dist;
          }
          ++cb;
        }
        ++ca;
      }
      ++csa;
      ++csb;
    }

    // Back track to find optimal path
    Candidate *track_cand=nullptr;
    double final_prob = -0.001;
    Point_Candidates& last_candidates = m_traj_candidates->back();
    for (
      Point_Candidates::iterator c = last_candidates.begin();
      c!=last_candidates.end();
      ++c
      )
    {
      // One more step here to filter out these with equal final probability
      if(final_prob < c->cumu_prob)
      {
        final_prob = c->cumu_prob;
        track_cand = &(*c);
      }
    }
    int i = N-1;
    opath.push_back(track_cand);
    // Iterate from tail to head to assign path
    while ((track_cand=track_cand->prev)!=NULL)
    {
      opath.push_back(track_cand);
    }
    std::reverse(opath.begin(), opath.end());
    return opath;
  };

  /**
   * Get the shortest path (SP) distance from Candidate ca to cb
   * @param  ca
   * @param  cb
   * @return  the SP from ca to cb
   */
  double get_sp_dist(Point_Candidates::iterator& ca,
                     Point_Candidates::iterator& cb)
  {
    double sp_dist=0;
    //  Transition on the same edge
    if ( ca->edge->id == cb->edge->id && ca->offset <= cb->offset )
    {
      sp_dist = cb->offset - ca->offset;
    }
    else if(ca->edge->target == cb->edge->source)
    {
      // Transition on the same OD nodes
      sp_dist = ca->edge->length - ca->offset + cb->offset;
    }
    else
    {
      // No sp path exist from O to D.
      Record *r = m_ubodt.look_up(ca->edge->target,cb->edge->source);
      sp_dist = r==NULL ? DISTANCE_NOT_FOUND : r->cost +
                ca->edge->length - ca->offset + cb->offset;
    }
    return sp_dist;
  };
  /**
   * Get the penalized shortest path (SP) distance from Candidate ca to cb
   */
  double get_sp_dist_penalized(Point_Candidates::iterator& ca,
                               Point_Candidates::iterator& cb,
                               double pf)
  {
    double sp_dist=0;
    //  Transition on the same edge
    if ( ca->edge->id == cb->edge->id && ca->offset <= cb->offset )
    {
      sp_dist = cb->offset - ca->offset;
    }
    else if(ca->edge->target == cb->edge->source)
    {
      // Transition on the same OD nodes
      sp_dist = ca->edge->length - ca->offset + cb->offset;
    }
    else
    {
      Record *r = m_ubodt.look_up(ca->edge->target,cb->edge->source);
      // No sp path exist from O to D.
      if (r==NULL) return DISTANCE_NOT_FOUND;
      // calculate original SP distance
      sp_dist = r->cost + ca->edge->length - ca->offset + cb->offset;
      // Two penalized cases
      if(r->prev_n==cb->edge->target)
      {
        sp_dist+=pf*cb->edge->length;
      }
      if(r->first_n==ca->edge->source)
      {
        sp_dist+=pf*ca->edge->length;
      }

    }
    return sp_dist;
  };
  /**
   *  Calculate the length of all segments in a linestring
   */
  static std::vector<double> cal_eu_dist(const LineString &trajectory)
  {
    int N = trajectory.getNumPoints();
    std::vector<double> lengths(N-1);
    double x0 = trajectory.getX(0);
    double y0 = trajectory.getY(0);
    for(int i = 1; i < N; ++i)
    {
      double x1 = trajectory.getX(i);
      double y1 = trajectory.getY(i);
      double dx = x1 - x0;
      double dy = y1 - y0;
      lengths[i-1]=sqrt(dx * dx + dy * dy);
      x0 = x1;
      y0 = y1;
    }
    return lengths;
  };

  Traj_Candidates *get_traj_candidates(){
    return m_traj_candidates;
  };

  std::vector<double> &get_eu_distances(){
    return eu_distances;
  }

private:
  // a pointer to trajectory candidates
  Traj_Candidates *m_traj_candidates;
  // reference to GPS trajectory
  const LineString &m_traj;
  // UBODT
  UBODT &m_ubodt;
  // Euclidean distances of segments in the trajectory
  std::vector<double> eu_distances;
  // This is the value returned as the SP distance if it is not found in UBODT
  float DISTANCE_NOT_FOUND;
};
}
#endif /* MM_TRANSITION_GRAPH_ROUTING_HPP */
