#include "config/gps_config.hpp"

namespace MM {

std::string GPSConfig::to_string() const{
  std::stringstream ss;
  ss<<"GPS file: "<< file << "\n";
  ss<<"Id: "<< id << "\n";
  ss<<"Geom: "<< geom << "\n";
  ss<<"X: "<< x << "\n";
  ss<<"Y: "<< y << "\n";
  ss<<"Timestamp: "<< timestamp << "\n";
  return ss.str();
};

GPSConfig GPSConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data){
  std::string file = tree.get<std::string>("fmm_config.input.gps.file");
  std::string id = tree.get("fmm_config.input.gps.id", "id");
  std::string geom = tree.get("fmm_config.input.gps.geom", "geom");
  std::string timestamp = tree.get("fmm_config.input.gps.timestamp",
  "timestamp");
  std::string x = tree.get("fmm_config.input.gps.x", "x");
  std::string y = tree.get("fmm_config.input.gps.y", "y");
  return GPSConfig{file,id,geom,x,y,timestamp};
};

GPSConfig GPSConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data){
  std::string file = arg_data["gps"].as<std::string>();
  std::string id = arg_data["gps_id"].as<std::string>();
  std::string geom = arg_data["gps_geom"].as<std::string>();
  std::string timestamp = arg_data["gps_timestamp"].as<std::string>();
  std::string x = arg_data["gps_x"].as<std::string>();
  std::string y = arg_data["gps_y"].as<std::string>();
  return GPSConfig{file,id,geom,x,y,timestamp};
};

}
