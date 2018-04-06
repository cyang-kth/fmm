/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// demo.cpp
//
// (C) Copyright 2002-4 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iomanip>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

struct Record
{
    int source;
    int target;
    int next_n; // next_n in the paper
    int prev_n;
    int next_e;
    double cost;
};

// serialize a record object

namespace boost{
    namespace serialization{
        template<class Archive>
        void serialize(Archive & ar, Record &r, const unsigned int version)
        {
            ar & r.source;
            ar & r.target;
            ar & r.next_n;
            ar & r.prev_n;
            ar & r.next_e;
            ar & r.cost;
        };
    }
}

int main(int argc, char *argv[])
{
    std::string inputfile(argv[1]);
    std::string outputfile(argv[2]);
    printf("Program start.\n");
    std::cout<<"Reading UBODT file from: " << inputfile << '\n';
    std::ifstream ifs(inputfile.c_str());
    std::ofstream ofs(outputfile.c_str());
    boost::archive::binary_oarchive oa(ofs);
    std::string line;
    if(std::getline(ifs,line)){
        std::cout<<"Header skipped \n";
        // printf("Header line skipped.\n");
    };
    int source,target,next_n,prev_n,next_e;
    double cost;
    int NUM_ROWS = 0;
    while (std::getline(ifs,line))
    {
        ++NUM_ROWS;
        if (NUM_ROWS%1000000==0) {
            std::cout<<"Progress "<<NUM_ROWS<<"\n";
        }
        /* Parse line into a record */
        sscanf(
            line.c_str(),"%d;%d;%d;%d;%d;%lf",
               &source,
               &target,
               &next_n,
               &prev_n,
               &next_e,
               &cost
        );
        Record r={source,target,next_n,prev_n,next_e,cost};
        oa << r;
    };
    ifs.close();
    ofs.close();
    printf("Number of rows read %d.\n",NUM_ROWS);
    printf("Program finish.\n");
};

