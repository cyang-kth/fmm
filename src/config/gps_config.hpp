//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_GPS_CONFIG_HPP_
#define FMM_SRC_CONFIG_GPS_CONFIG_HPP_

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "cxxopts/cxxopts.hpp"


namespace MM{

struct GPSConfig{
  std::string file;
  std::string id;
  std::string geom;
  std::string x;
  std::string y;
  std::string timestamp;
  bool gps_point = false; // The GPS data stores point or not
  bool validate() const;
  void print() const;
  // Check gps format, returns 0 for GDAL shapefile,
  // 1 for trajectory CSV file, 2 for GPS point CSV file
  // -1 if not other format
  int get_gps_format() const;
  static GPSConfig load_from_xml(const boost::property_tree::ptree &xml_data);
  static GPSConfig load_from_arg(const cxxopts::ParseResult &arg_data);
};



}

#endif //FMM_SRC_CONFIG_GPS_CONFIG_HPP_
