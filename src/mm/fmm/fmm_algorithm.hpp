//
// Created by Can Yang on 2020/3/22.
//

#ifndef MM_SRC_MM_FMM_FMM_ALGORITHM_H_
#define MM_SRC_MM_FMM_FMM_ALGORITHM_H_

#include "mm/transition_graph.hpp"
#include "mm/fmm/ubodt.hpp"
#include "mm/fmm/fmm_config.hpp"

namespace MM {

class FMM{
public:
  FMM(const Network &network, const NetworkGraph &graph,
      const UBODT &ubodt)
      : network_(network), graph_(graph), ubodt_(ubodt){
  };
  // Procedure of HMM based map matching algorithm.
  MatchResult match_temporal_traj(const TemporalTrajectory &traj,
                                  const FMMConfig &config);
protected:
  double get_sp_dist(const Candidate* ca,
                     const Candidate* cb);
  // Update the transition graph
  void update_tg(TransitionGraph *tg,
                 const TemporalTrajectory &traj,
                 const MMConfig &config);

  void update_layer(int level, TGLayer *la_ptr, TGLayer *lb_ptr,
                    double eu_dist);
private:
  const Network &network_;
  const NetworkGraph &graph_;
  const UBODT &ubodt_;
};

}

#endif //MM_SRC_MM_FMM_FMM_H_
