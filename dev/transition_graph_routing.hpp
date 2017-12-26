#ifndef MM_TRANSITION_GRAPH_ROUTING_HPP
#define MM_TRANSITION_GRAPH_ROUTING_HPP
#include "../src/types.hpp"
#include "../src/network.hpp"
#include "ubodt_routing.hpp"
#include "float.h"
#include "../src/multilevel_debug.h"
namespace MM
{
/**
 *  This class is designed for optimal path inference in HMM-MM algorithm
 *  where Viterbi algorithm is implemented.
 *
 *  It uses an UBODT + Dijkstra.
 */
class TransitionGraphRouting
{
public:
    static constexpr float NO_GAP_DIST = FLT_MAX;
    TransitionGraphRouting(Traj_Candidates *traj_candidates,OGRLineString *traj,UBODT_Routing *ubodt):
        m_traj_candidates(traj_candidates),
        m_traj(traj),
        m_ubodt(ubodt),
        eu_distances(cal_eu_dist(traj))
    {};
    /**
        The pointer returned should be freed by the caller.
    */
    O_Path *viterbi(double pf=0,bool routing_query=false)
    {
        OPI_DEBUG(2) std::cout<<"-----------------------"<<std::endl;
        OPI_DEBUG(2) std::cout<<"Viterbi start!"<<std::endl;
        if (m_traj_candidates->empty()) return nullptr;
        int N = m_traj_candidates->size();
        O_Path *opt_path = new O_Path(N);
        /* Update transition probabilities */
        Traj_Candidates::iterator csa = m_traj_candidates->begin();
        Traj_Candidates::iterator csb = m_traj_candidates->begin();
        ++csb;
        OPI_DEBUG(2) std::cout<<"step;from;to;sp;eu;tran_prob;e_prob;cumu_prob"<<std::endl;
        while (csb != m_traj_candidates->end())
        {
            Point_Candidates::iterator ca = csa->begin();
            double eu_dist=eu_distances[std::distance(m_traj_candidates->begin(),csa)];
            while (ca != csa->end())
            {
                Point_Candidates::iterator cb = csb->begin();
                while (cb != csb->end())
                {
                    int step =std::distance(m_traj_candidates->begin(),csa); // The problem seems to be here
                    // Calculate transition probability
                    double sp_dist;
                    if (pf!=0){
                        sp_dist = get_sp_dist_penalized(ca,cb,pf);
                    } else {
                        if (routing_query){
                            sp_dist = get_sp_dist_routing(ca,cb);
                        } else {
                            sp_dist = get_sp_dist_norouting(ca,cb);
                        }
                    }
                    /*
                        A degenerate case is that the same point
                        is reported multiple times where both eu_dist and sp_dist = 0
                    */
                    double tran_prob =eu_dist>sp_dist?sp_dist/eu_dist:eu_dist/(sp_dist+0.00001);
                    if (ca->cumu_prob + tran_prob * cb -> obs_prob >= cb->cumu_prob)
                    {
                        cb->cumu_prob = ca->cumu_prob + tran_prob * cb->obs_prob;
                        cb->prev = &(*ca);
                    }
                    OPI_DEBUG(2) std::cout<<step <<";"<<ca->edge->id_attr<<";"<<cb->edge->id_attr<<";"<<sp_dist<<";"<<eu_dist<<";"<<tran_prob<<";"<<cb->obs_prob<<";"<<ca->cumu_prob + tran_prob * cb->obs_prob<<std::endl;
                    ++cb;
                }
                ++ca;
            }
            ++csa;
            ++csb;
        } // End of calculating transition probability

        // Back track to find optimal path
        OPI_DEBUG(2) std::cout<<"Find last optimal candidate"<<std::endl;
        Candidate *track_cand;
        double final_prob = 0;
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
        OPI_DEBUG(2) std::cout<<"Back tracking"<<std::endl;
        int i = N-1;
        (*opt_path)[i]=track_cand;
        OPI_DEBUG(2) std::cout<<"Optimal Path candidate index "<<i<<" edge id "<<track_cand->edge->id_attr<<std::endl;
        // Iterate from tail to head to assign path
        while ((track_cand=track_cand->prev)!=NULL)
        {
            (*opt_path)[i-1]=track_cand;
            OPI_DEBUG(2) std::cout<<"Optimal Path candidate index "<< i-1 <<" edge_id "<<track_cand->edge->id_attr<<std::endl;
            --i;
        }
        OPI_DEBUG(2) std::cout<<"Viterbi ends"<<std::endl;
        return opt_path;
    };
    /**
        Three cases are checked:
            1. Transition on the same edge
            2. Transition on the same OD nodes
            3. No sp path exist from O to D.
    */
    double get_sp_dist_norouting(Point_Candidates::iterator& ca,Point_Candidates::iterator& cb)
    {
        double sp_dist=0;
        //  Transition on the same edge
        if ( ca->edge->id == cb->edge->id && ca->offset < cb->offset )
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
            sp_dist = m_ubodt->get_sp_distance_norouting(ca->edge->target,
            cb->edge->source) + ca->edge->length - ca->offset + cb->offset;
        }
        return sp_dist;
    };
    double get_sp_dist_routing(Point_Candidates::iterator& ca,Point_Candidates::iterator& cb)
    {
        double sp_dist=0;
        //  Transition on the same edge
        if ( ca->edge->id == cb->edge->id && ca->offset < cb->offset )
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
            sp_dist = m_ubodt->get_sp_distance_routing(ca->edge->target,
            cb->edge->source) + ca->edge->length - ca->offset + cb->offset;
        }
        return sp_dist;
    };
    /**
        Add penalization to the shortest path distance.
    */
    double get_sp_dist_penalized(Point_Candidates::iterator& ca,Point_Candidates::iterator& cb,double pf)
    {
        double sp_dist=0;
        //  Transition on the same edge
        if ( ca->edge->id == cb->edge->id && ca->offset < cb->offset )
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
            int prev_n;
            int first_n;
            double cost;
            m_ubodt->get_sp_distance_first_prev_node(ca->edge->target,
                cb->edge->source,
                &first_n,&prev_n,&cost);
            // calculate original SP distance
            sp_dist = cost + ca->edge->length - ca->offset + cb->offset;
            // Two penalized cases
            if(prev_n==cb->edge->target)
            {
                sp_dist+=pf*cb->edge->length;
            }
            if(first_n==ca->edge->source)
            {
                sp_dist+=pf*ca->edge->length;
            }
        }
        return sp_dist;
    };
    /* Calculate eu_clidean distances of all segments of a linestring
     * https://github.com/libgeos/libgeos/blob/99f187fb3fd777440acb51fff371d20a80e6e1ed/src/algorithm/CGAlgorithms.cpp
     */
    static std::vector<double> cal_eu_dist(OGRLineString *trajectory)
    {
        OPI_DEBUG(2) std::cout<<"Calculating lengths of segments"<<std::endl;
        int N = trajectory->getNumPoints();
        std::vector<double> lengths(N-1);
        double x0 = trajectory->getX(0);
        double y0 = trajectory->getY(0);
        for(int i = 1; i < N; ++i)
        {
            double x1 = trajectory->getX(i);
            double y1 = trajectory->getY(i);
            double dx = x1 - x0;
            double dy = y1 - y0;
            lengths[i-1]=sqrt(dx * dx + dy * dy); // Not push_back which will insert new elements.
            x0 = x1;
            y0 = y1;
        }
        return lengths;
    };
private:
    Traj_Candidates *m_traj_candidates;
    OGRLineString *m_traj;
    UBODT_Routing *m_ubodt; // used for transition calculation
    std::vector<double> eu_distances;
};
}
#endif /* MM_TRANSITION_GRAPH_ROUTING_HPP */
