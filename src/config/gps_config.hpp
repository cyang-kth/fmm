//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_GPS_CONFIG_HPP_
#define FMM_SRC_CONFIG_GPS_CONFIG_HPP_

#include<string>

namespace MM{

struct GPSConfig{
  std::string gps_file;
  std::string gps_id;
  std::string gps_geom;
  std::string gps_timestamp;
  std::string gps_x;
  std::string gps_y;
  bool gps_point = false; // The GPS data stores point or not
  static GPSConfig load_from_xml(){

  };
  static GPSConfig load_from_arg(){

  };
};



}

#endif //FMM_SRC_CONFIG_GPS_CONFIG_HPP_
