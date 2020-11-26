#include "shapefil.h"
#include <iostream>

int main(int argc, char **argv){
  SHPHandle handle = SHPOpen(argv[1],"rb");
  int feature_count = 0;
  int geometry_type = 0;
  SHPGetInfo(handle,&feature_count,&geometry_type, NULL, NULL);
  std::cout.precision(10);
  std::cout<<"Number of features "<< feature_count <<"\n";
  std::cout<<"Geometry type "<< geometry_type <<"\n";
  SHPObject *feature = SHPReadObject(handle, 0);
  int nVertices = feature->nVertices;
  std::cout<<"Number of vertices is "<< nVertices <<"\n";
  for (int i = 0; i<nVertices;++i){
    std::cout<< " Point " << i
             << " X "<< *(feature->padfX+i)
             << " Y " << *(feature->padfY+i) <<"\n";
  }
  SHPDestroyObject(feature);
  SHPClose(handle);
};
