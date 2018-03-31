#ifndef MM_UBODT_ROUTING_HPP
#define MM_UBODT_ROUTING_HPP
#include "../src/types.hpp"
#include "../src/network.hpp"
#include "network_graph.hpp"
#include "../src/ubodt.hpp"
#include "float.h"
#include "../src/multilevel_debug.h"

namespace MM
{
/**
 *  This class differs from UBODT in the way that a network
 *  graph is used for query of distances larger than delta.
 *
 *
 *  Method names end with "from_routing" means that the graph is
 *  invoked to find the shortest path.
 */
class UBODT_Routing
{
public:
    UBODT_Routing(int multiplier,int hash_value,NetworkGraph *g,double delta=5000):
        MULTIPLIER(multiplier),NHASH(hash_value),
        m_g(g), LARGE_GAP_DISTANCE(delta){};
    void read_csv(const std::string &filename)
    {
        std::cout<<"Reading UBODT file from: " << filename << '\n';
        hashtable = (record **) malloc(sizeof(record*)*NHASH);
        /* This initialization is required to later free the memory, to figure out the problem */
        for (int i = 0; i < NHASH; i++)
        {
            hashtable[i] = NULL;
        }
        FILE* stream = fopen(filename.c_str(), "r");
        int NUM_ROWS = 0;
        char line[BUFFER_LINE];
        if(fgets(line, BUFFER_LINE, stream))
        {
            printf("    Header line skipped.\n");
        };
        while (fgets(line, BUFFER_LINE, stream))
        {
            ++NUM_ROWS;
            record *r =(record *) malloc(sizeof(record));
            /* Parse line into a record */
            sscanf(
                line,"%d;%d;%d;%d;%d;%lf",
                &r->source,
                &r->target,
                &r->first_n,
                &r->prev_n,
                &r->next_e,
                &r->cost
            );
            r->next=NULL;
            if (NUM_ROWS%1000000==0) printf("Read rows: %d\n",NUM_ROWS);
            /* Insert into the hash table */
            insert(r);
        };
        fclose(stream);
        printf("    Number of rows read %d.\n",NUM_ROWS);
        double lf = NUM_ROWS/(double)NHASH;
        printf("    Estimated load factor #elements/#tablebuckets %lf.\n",lf);
        if (lf>10) printf("    *** Warning, load factor is too large.\n");
        printf("Finish reading UBODT.\n");
    };
    /**
     *  To implement this method
     */
    double get_sp_distance_norouting(int source,int target)
    {
        int h = (source*MULTIPLIER+target)%NHASH;
        record *r = hashtable[h];
        while (r != NULL)
        {
            if (r->source==source && r->target==target)
            {
                return r->cost;
            }
            else
            {
                r=r->next;
            }
        }
        if (r==NULL)
        {
            // Assume that the distance is LARGE_GAP_DISTANCE
            return LARGE_GAP_DISTANCE;
        }
    };

    double get_sp_distance_routing(int source,int target)
    {
        int h = (source*MULTIPLIER+target)%NHASH;
        record *r = hashtable[h];
        while (r != NULL)
        {
            if (r->source==source && r->target==target)
            {
                return r->cost;
            }
            else
            {
                r=r->next;
            }
        }
        if (r==NULL)
        {
            // std::cout<<"Routing invoked source "<<source <<" target "<<target<<'\n';
            ++routing_distance_called;
            double distance = m_g->single_source_target_distance(source,target);
            return distance;
        }
    };

    bool get_sp_distance_first_prev_node(int source,int target,int *first_n,int *prev_n,double *cost)
    {
        int h = (source*MULTIPLIER+target)%NHASH;
        record *r = hashtable[h];
        while (r != NULL)
        {
            if (r->source==source && r->target==target)
            {
                *first_n = r->first_n;
                *prev_n = r->prev_n;
                *cost = r->cost;
            }
            else
            {
                r=r->next;
            }
        }
        if (r==NULL)
        {
            // Invoke the raw routing
            ++routing_distance_called;
            return m_g->single_source_target_with_first_prev_node(source,target,first_n,prev_n,cost);
        }
    };

    /*
        Return a shortest path containing edges from source to target,
        where source != target.
    */
    std::vector<int> look_sp_path_norouting(int source,int target)
    {
        CPC_DEBUG(4) std::cout<<"Look shortest path from "<< source <<" to "<<target<<'\n';
        std::vector<int> edges;
        if (source==target) return edges;
        record *r=look_up_record(source,target);
        // No transition exist from source to target
        if (r==NULL) return edges;
        while(r->first_n!=target)
        {
            edges.push_back(r->next_e);
            r=look_up_record(r->first_n,target);
        }
        edges.push_back(r->next_e);
        return edges;
    };
    std::vector<int> look_sp_path_routing(int source,int target)
    {
        CPC_DEBUG(4) std::cout<<"Look shortest path from "<< source <<" to "<<target<<'\n';
        std::vector<int> edges;
        if (source==target) return edges;
        record *r=look_up_record(source,target);
        // No transition exist from source to target
        if (r==NULL)
        {
            ++routing_path_called;
            return m_g->single_source_target_path(source,target);
        }
        while(r->first_n!=target)
        {
            edges.push_back(r->next_e);
            r=look_up_record(r->first_n,target);
        }
        edges.push_back(r->next_e);
        return edges;
    };

    /**
        If there exist a break between consecutive candidates, implying
        that there is no path exist for transition in UBODT, thus the
        program should be terminated.
        The returned value should be deleted by the caller.
    */
    C_Path *construct_complete_path_norouting(O_Path *path)
    {
        CPC_DEBUG(2) std::cout<<"-----------------------"<<'\n';
        CPC_DEBUG(2) std::cout<<"Construct complete path"<<'\n';
        if (path==nullptr) return nullptr;
        C_Path *edges= new C_Path();
        int N = path->size();
        edges->push_back((*path)[0]->edge->id);
        for(int i=0; i<N-1; ++i)
        {
            Candidate* a = (*path)[i];
            Candidate* b = (*path)[i+1];
            if ((a->edge->id!=b->edge->id) || (a->offset>b->offset))
            {
                auto segs = look_sp_path_norouting(a->edge->target,b->edge->source);
                // No transition exist in UBODT
                if (segs.empty() &&  a->edge->target!=b->edge->source)
                {
                    CPC_DEBUG(2) std::cout<<"Large gap from "<< a->edge->target <<" to "<< b->edge->source <<'\n';
                    CPC_DEBUG(2) std::cout<<"Construct complete path skipped"<<'\n';
                    delete edges; // free the memory of edges
                    return nullptr;
                }
                for (int e:segs)
                {
                    edges->push_back(e);
                }
                edges->push_back(b->edge->id);
            }
        }
        CPC_DEBUG(2) std::cout<<"Construct complete path finished "<<'\n';
        return edges;
    };
    C_Path *construct_complete_path_routing(O_Path *path)
    {
        CPC_DEBUG(2) std::cout<<"-----------------------"<<'\n';
        CPC_DEBUG(2) std::cout<<"Construct complete path"<<'\n';
        if (path==nullptr) return nullptr;
        C_Path *edges= new C_Path();
        int N = path->size();
        edges->push_back((*path)[0]->edge->id);
        for(int i=0; i<N-1; ++i)
        {
            Candidate* a = (*path)[i];
            Candidate* b = (*path)[i+1];
            if ((a->edge->id!=b->edge->id) || (a->offset>b->offset))
            {
                auto segs = look_sp_path_routing(a->edge->target,b->edge->source);
                // No path exist.
                if (segs.empty() &&  a->edge->target!=b->edge->source)
                {
                    CPC_DEBUG(2) std::cout<<"Large gap from "<< a->edge->target <<" to "<< b->edge->source <<'\n';
                    CPC_DEBUG(2) std::cout<<"Construct complete path skipped"<<'\n';
                    delete edges; // free the memory of edges
                    return nullptr;
                }
                for (int e:segs)
                {
                    edges->push_back(e);
                }
                edges->push_back(b->edge->id);
            }
        }
        CPC_DEBUG(2) std::cout<<"Construct complete path finished "<<'\n';
        return edges;
    };

    /**
     *  Print statistics of the hashtable to a file
     */
    void print_statictics(const char*filename)
    {
        /*
            Iterate through all buckets to get statistics
            Bucket size, counts
        */
        std::map<int,int> statistics;
        for (int i=0; i<NHASH; ++i)
        {
            int count=0;
            record *r=hashtable[i];
            while (r!=NULL)
            {
                r=r->next;
                ++count;
            }
            statistics[count]=statistics[count]+1;
        }
        std::ofstream outputfile(filename);
        if (outputfile.is_open())
        {
            outputfile<<"BucketElements;Counts\n";
            for (std::map<int,int>::iterator it=statistics.begin(); it!=statistics.end(); ++it)
                outputfile<< it->first << ";" << it->second << '\n';
            outputfile.close();
        }
        else std::cout << "Unable to write statistics to file"<<'\n';
    };
    ~UBODT_Routing()
    {
        /* Clean hashtable */
        std::cout<< "Clean UBODT" << '\n';
        int i;
        for (i=0; i<NHASH; ++i)
        {
            record* head = hashtable[i];
            record* curr;
            while ((curr = head) != NULL)   // set curr to head, stop if list empty.
            {
                head = head->next;          // advance head to next element.
                free(curr);        // delete saved pointer.
            }
        }
        // Destory hash table pointer
        free(hashtable);
        std::cout<< "Clean UBODT finished" << '\n';
    };
    int get_routing_path_times()
    {
        return routing_path_called;
    };
    int get_routing_distance_times()
    {
        return routing_distance_called;
    };

private:
    record *look_up_record(int source,int target)
    {
        int h = (source*MULTIPLIER+target)%NHASH;
        record *r = hashtable[h];
        while (r != NULL)
        {
            if (r->source==source && r->target==target)
            {
                return r;
            }
            else
            {
                r=r->next;
            }
        }
        //printf("Not found s %d t %d h %d\n",source,target,h); NULL will be returned.
        return r;
    };
    void insert(record *r)
    {
        int h = (r->source*MULTIPLIER+r->target)%NHASH ;
        r->next = hashtable[h];
        hashtable[h]=r;
    };
    static const int BUFFER_LINE = 1024;
    const int NHASH;
    const int MULTIPLIER;
    int routing_distance_called=0;
    int routing_path_called=0;
    //int maxnode=0;
    record** hashtable;
    NetworkGraph *m_g;
    double LARGE_GAP_DISTANCE;
}; //UBODT_Routing
}
#endif /* MM_UBODT_ROUTING_HPP */
