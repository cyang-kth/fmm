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
struct Record
{
    int source;
    int target;
    int next_n; // next_n in the paper
    int prev_n;
    int next_e;
    double cost;
};
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
    ofs<<"source;target;next_n;prev_n;next_e;distance\n";
    int source,target,next_n,prev_n,next_e;
    double cost;
    int NUM_ROWS = 0;
    std::streampos archiveOffset = ifs.tellg(); 
    std::streampos streamEnd = ifs.seekg(0, std::ios_base::end).tellg();
    ifs.seekg(archiveOffset);
    boost::archive::binary_iarchive ia(ifs);
    Record r;
    while (ifs.tellg() < streamEnd)
    {
        ++NUM_ROWS;
        ia >> r;
        ofs << r.source << ";" << r.target << ";" << r.next_n << ";"
                       << r.prev_n << ";" << r.next_e << ";" << r.cost
                       << "\n";
    }
    ifs.close();
    ofs.close();
    printf("Number of rows exported %d.\n",NUM_ROWS);
    printf("Program finish.\n");
};

