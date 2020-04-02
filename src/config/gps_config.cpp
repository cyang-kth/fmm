#include "config/gps_config.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"

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
  GPSConfig config;
  config.file = xml_data.get<std::string>("mm_config.input.gps.file");
  config.id = xml_data.get("mm_config.input.gps.id", "id");
  config.geom = xml_data.get("mm_config.input.gps.geom", "geom");
  config.timestamp = xml_data.get("mm_config.input.gps.timestamp",
  "timestamp");
  config.x = xml_data.get("mm_config.input.gps.x", "x");
  config.y = xml_data.get("mm_config.input.gps.y", "y");
  return config;
};

GPSConfig GPSConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data){
  GPSConfig config;
  config.file = arg_data["gps"].as<std::string>();
  config.id = arg_data["gps_id"].as<std::string>();
  config.geom = arg_data["gps_geom"].as<std::string>();
  config.timestamp = arg_data["gps_timestamp"].as<std::string>();
  config.x = arg_data["gps_x"].as<std::string>();
  config.y = arg_data["gps_y"].as<std::string>();
  return config;
};

int GPSConfig::get_gps_format() const {
  std::string fn_extension = file.substr(
      file.find_last_of(".") + 1);
  if (fn_extension == "csv" || fn_extension == "txt") {
    if (gps_point) {
      return 2;
    } else {
      return 1;
    }
  } else if (fn_extension == "gpkg" || fn_extension == "shp") {
    return 0;
  } else {
    SPDLOG_CRITICAL("GPS file extension {} unknown",fn_extension);
    return -1;
  }
};

bool GPSConfig::validate() const {
  if (!UTIL::file_exists(file))
  {
    SPDLOG_CRITICAL("GPS file {} not found",file);
    return false;
  };
  if (get_gps_format()<0) {
    SPDLOG_CRITICAL("Unknown GPS format");
    return false;
  }
  return true;
}

}
