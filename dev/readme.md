### FMM functions under development

#### FMM in UBODT + Dijkstra(Optimized) mode

The app `fmm_ubodt_dijkstra.cpp` contains an optimized integration of UBODT with Dijkstra in map matching (corresponding to mode `M2` in the fmm paper). When the shortest path distance between consecutive probes is not found in UBODT, it will return delta as an estimation. In case of a true large gap, it will try to invoke Dijkstra. 

Install it with

    make install

#### Implementation details

In `fmm_ubodt_dijkstra.cpp` file, we create a `graph` and register it in the `ubodt` and register `ubodt` in a transition graph `tg`.

    NetworkGraph graph(&network); // construct a graph 
    UBODT_Routing ubodt(multiplier,NHASH,&graph,config.delta); // register the graph in ubodt
    TransitionGraphRouting tg(&traj_candidates,trajectory.geom,&ubodt); // register ubodt in the trasition graph

It will first return `delta` as an estimation, if optimal path is found but the complete path is not found (implying a true large gap), it will turn to Dijkstra. 

    O_Path *o_path_ptr = tg.viterbi(config.penalty_factor); // Default is ubodt
    // Complete path construction as an array of indices of edges vector
    C_Path *c_path_ptr = ubodt.construct_complete_path_norouting(o_path_ptr);
    // If it is a true large gap
    if (o_path_ptr!=nullptr && c_path_ptr==nullptr){
        o_path_ptr = tg.viterbi(config.penalty_factor,true); // Run viterbi with ubodt+dijkstra
        c_path_ptr = ubodt.construct_complete_path_routing(o_path_ptr);
    }