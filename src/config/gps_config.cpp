#include "config/gps_config.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"

void FMM::CONFIG::GPSConfig::print() const{
  int format = get_gps_format();
  if (format==0) {
    SPDLOG_INFO("GPS format: GDAL trajectory");
    SPDLOG_INFO("File name: {} ",file);
    SPDLOG_INFO("ID name: {} ",id);
    SPDLOG_INFO("Timestamp name: {} ",timestamp);
  } else if (format==1) {
    SPDLOG_INFO("GPS format: CSV trajectory");
    SPDLOG_INFO("File name: {} ",file);
    SPDLOG_INFO("ID name: {} ",id);
    SPDLOG_INFO("Geom name: {} ",geom);
    SPDLOG_INFO("Timestamp name: {} ",timestamp);
  } else {
    SPDLOG_INFO("GPS format: CSV point");
    SPDLOG_INFO("File name: {} ",file);
    SPDLOG_INFO("ID name: {} ",id);
    SPDLOG_INFO("x name: {} ",x);
    SPDLOG_INFO("y name: {} ",y);
    SPDLOG_INFO("Timestamp name: {} ",timestamp);
  }
};

FMM::CONFIG::GPSConfig FMM::CONFIG::GPSConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data){
  GPSConfig config;
  config.file = xml_data.get<std::string>("config.input.gps.file");
  config.id = xml_data.get("config.input.gps.id", "id");
  config.geom = xml_data.get("config.input.gps.geom", "geom");
  config.timestamp = xml_data.get("config.input.gps.timestamp",
  "timestamp");
  config.x = xml_data.get("config.input.gps.x", "x");
  config.y = xml_data.get("config.input.gps.y", "y");
  config.gps_point = !(!xml_data.get_child_optional(
      "config.input.gps.gps_point"));
  return config;
};

FMM::CONFIG::GPSConfig FMM::CONFIG::GPSConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data){
  GPSConfig config;
  config.file = arg_data["gps"].as<std::string>();
  config.id = arg_data["gps_id"].as<std::string>();
  config.geom = arg_data["gps_geom"].as<std::string>();
  config.timestamp = arg_data["gps_timestamp"].as<std::string>();
  config.x = arg_data["gps_x"].as<std::string>();
  config.y = arg_data["gps_y"].as<std::string>();
  if (arg_data.count("gps_point")>0)
    config.gps_point = true;
  return config;
};

int FMM::CONFIG::GPSConfig::get_gps_format() const {
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

bool FMM::CONFIG::GPSConfig::validate() const {
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
