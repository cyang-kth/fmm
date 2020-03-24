//
// Created by Can Yang on 2020/3/22.
//

#ifndef MM_SRC_MM_FMM_FMM_H_
#define MM_SRC_MM_FMM_FMM_H_

#include "mm/mm_interface.hpp"
#include "mm/composite_graph.hpp"
#include "mm/transition_graph.hpp"
#include "ubodt.h"

namespace MM {

class FMM : public MMInterface {
 public:
  FMM(const Network &network_arg, const NetworkGraph &ng_arg,
      const UBODT &ubodt_arg)
      : MMInterface(network_arg, ng_arg), ubodt(ubodt_arg) {
  };

  // Procedure of HMM based map matching algorithm.
  MatchResult match_temporal_traj(const TemporalTrajectory &traj,
                                  const MMConfig &config);
 protected:
  double get_sp_dist(const Candidate* ca,
                     const Candidate* cb);
  void update_tg(TransitionGraph *tg,
                 const TemporalTrajectory &traj,
                 const MMConfig &config);

  void update_layer(int level, TGLayer *la_ptr, TGLayer *lb_ptr,
                    double eu_dist);
 private:
  const UBODT &ubodt;
};

}

#endif //MM_SRC_MM_FMM_FMM_H_
