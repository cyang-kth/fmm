/**
 * Content
 * Definition of UBODT, which is a hashtable containing the precomputed shortest path
 * routing results.
 *      
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_UBODT_HPP
#define MM_UBODT_HPP
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <map> /* used for statistics */
#include <fstream>
#include <boost/archive/binary_iarchive.hpp>
#include "types.hpp"
#include "multilevel_debug.h"
namespace MM
{
class UBODT
{
public:
    /**
     *  Constructor of UBODT
     *  @param  multiplier: used to calculate ID of an OD pair as n_o * multiplier + n_d 
     *  @param  hash_value: the number of buckets in the hashtable 
     */
    UBODT(int multiplier,int hash_value):MULTIPLIER(multiplier),NHASH(hash_value) {};

    /**
     * Read UBODT from a CSV file, which should be generated from the ubodt_gen program in format
     * 
     * source;target;next_n;last_n;next_e;distance
     * 1;2;2;1;0;1
     * 
     */
    void read_csv(const std::string &filename)
    {
        std::cout<<"Reading UBODT file (CSV format) from: " << filename << '\n';
        hashtable = (record **) malloc(sizeof(record*)*NHASH);
        /* This initialization is required to later free the memory, to figure out the problem */
        for (int i = 0; i < NHASH; i++){
            hashtable[i] = NULL;
        }
        FILE* stream = fopen(filename.c_str(), "r");
        int NUM_ROWS = 0;
        char line[BUFFER_LINE];
        if(fgets(line, BUFFER_LINE, stream)){
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
        std::cout<<"    Number of rows read " << NUM_ROWS << '\n';
        double lf = NUM_ROWS/(double)NHASH;
        std::cout<<"    Estimated load factor #elements/#tablebuckets "<<lf<<"\n";
        if (lf>10) std::cout<<"    *** Warning, load factor is too large.\n";
        std::cout<<"Finish reading UBODT.\n";
    };
    /**
     * Read ubodt from a binary file
     */
    void read_binary(const std::string &filename)
    {
        std::cout<<"Reading UBODT file (CSV binary) from: " << filename << '\n';
        hashtable = (record **) malloc(sizeof(record*)*NHASH);
        /* This initialization is required to later free the memory, to figure out the problem */
        for (int i = 0; i < NHASH; i++){
            hashtable[i] = NULL;
        }
        int NUM_ROWS = 0;
        std::ifstream ifs(filename.c_str());
        // Check byte offset
        std::streampos archiveOffset = ifs.tellg(); 
        std::streampos streamEnd = ifs.seekg(0, std::ios_base::end).tellg();
        ifs.seekg(archiveOffset);
        boost::archive::binary_iarchive ia(ifs);
        while (ifs.tellg() < streamEnd)
        {
            ++NUM_ROWS;
            record *r =(record *) malloc(sizeof(record));
            ia >> r->source;
            ia >> r->target;
            ia >> r->first_n;
            ia >> r->prev_n;
            ia >> r->next_e;
            ia >> r->cost;
            r->next=NULL;
            if (NUM_ROWS%1000000==0) printf("Read rows: %d\n",NUM_ROWS);
            /* Insert into the hash table */
            insert(r);
        }
        ifs.close();
        std::cout<<"    Number of rows read " << NUM_ROWS << '\n';
        double lf = NUM_ROWS/(double)NHASH;
        std::cout<<"    Estimated load factor #elements/#tablebuckets "<<lf<<"\n";
        if (lf>10) std::cout<<"    *** Warning, load factor is too large.\n";
        std::cout<<"Finish reading UBODT.\n";
    };
    record *look_up(int source,int target)
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
    /**
     *  Return a shortest path (SP) containing edges from source to target.
     *  In case that SP is not found, empty is returned. 
     */
    std::vector<int> look_sp_path(int source,int target){
        CPC_DEBUG(4) std::cout<<"Look shortest path from "<< source <<" to "<<target<<'\n';
        std::vector<int> edges;
        if (source==target) {return edges;}
        record *r=look_up(source,target);
        // No transition exist from source to target
        if (r==NULL){return edges;}
        while(r->first_n!=target){
            edges.push_back(r->next_e);
            r=look_up(r->first_n,target);
        }
        edges.push_back(r->next_e);
        return edges;
    };

    /**
     * Construct the complete path (a vector of edge ID) from an optimal path (a vector of candidates)
     * 
     * @param  path, an optimal path
     * @return  a pointer to a complete path, which should be freed by the caller. If there is a large 
     * gap in the optimal path implying complete path cannot be found in UBDOT, nullptr is returned
     */
    C_Path *construct_complete_path(O_Path *path){
        CPC_DEBUG(2) std::cout<<"-----------------------"<<'\n';
        CPC_DEBUG(2) std::cout<<"Construct complete path"<<'\n';
        if (path==nullptr) return nullptr;
        C_Path *edges= new C_Path();
        int N = path->size();
        edges->push_back((*path)[0]->edge->id);
        for(int i=0;i<N-1;++i){
            Candidate* a = (*path)[i];
            Candidate* b = (*path)[i+1];
            if ((a->edge->id!=b->edge->id) || (a->offset>b->offset)) {
                auto segs = look_sp_path(a->edge->target,b->edge->source);
                // No transition exist in UBODT
                if (segs.empty() &&  a->edge->target!=b->edge->source){
                CPC_DEBUG(1) std::cout<<"Large gap from "<< a->edge->target <<" to "<< b->edge->source <<'\n';
                CPC_DEBUG(1) std::cout<<"Construct complete path skipped"<<'\n';
                    delete edges; // free the memory of edges
                    return nullptr;
                }
                for (int e:segs){
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
    void print_statictics(const char*filename){
        /*
            Iterate through all buckets to get statistics
            Bucket size, counts
        */
        std::map<int,int> statistics;
        for (int i=0;i<NHASH;++i){
            int count=0;
            record *r=hashtable[i];
            while (r!=NULL){
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
    ~UBODT(){
        /* Clean hashtable */
        std::cout<< "Clean UBODT" << '\n';
        int i;
        for (i=0;i<NHASH;++i){
            record* head = hashtable[i];
            record* curr;
            while ((curr = head) != NULL) { // set curr to head, stop if list empty.
                head = head->next;          // advance head to next element.
                free(curr);        // delete saved pointer.
            }
        }
        // Destory hash table pointer
        free(hashtable);
        std::cout<< "Clean UBODT finished" << '\n';
    };
private:
    // Insert a record into the hash table
    void insert(record *r)
    {
        int h = (r->source*MULTIPLIER+r->target)%NHASH ;
        r->next = hashtable[h];
        hashtable[h]=r;
    };
    static const int BUFFER_LINE = 1024;
    const long long MULTIPLIER; // multipler to get a unique ID
    const int NHASH; // number of buckets
    //int maxnode=0;
    record** hashtable;
};
}
#endif /* MM_UBODT_HPP */
