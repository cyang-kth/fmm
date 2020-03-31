#include "mm/fmm/fmm_config.hpp"
#include "util/debug.hpp"
#include "util/util.hpp"

namespace MM {

std::string FMMConfig::to_string() const {
  std::stringstream ss;
  ss<<"k: "<< k << "\n";
  ss<<"radius: "<< radius << "\n";
  ss<<"gps_error: "<< gps_error << "\n";
  return ss.str();
};

FMMConfig FMMConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data){
  int k = xml_data.get("fmm_config.parameters.k", 8);
  double radius = xml_data.get("fmm_config.parameters.r", 300.0);
  double gps_error = xml_data.get("fmm_config.parameters.gps_error", 50.0);
  return FMMConfig{k,radius,gps_error};
};

FMMConfig FMMConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data){
  int k = arg_data["candidates"].as<int>();
  double radius = arg_data["radius"].as<double>();
  double gps_error = arg_data["error"].as<double>();
  return FMMConfig{k,radius,gps_error};
};

}
