#include "mm/stmatch/stmatch_config.hpp"

namespace MM {

std::string STMATCHConfig::to_string() const {
  std::stringstream ss;
  ss<<"k: "<< k << "\n";
  ss<<"radius: "<< radius << "\n";
  ss<<"gps_error: "<< gps_error << "\n";
  ss<<"vmax: "<< vmax << "\n";
  ss<<"factor: "<< factor << "\n";
  return ss.str();
};

STMATCHConfig STMATCHConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data){
  int k = xml_data.get("fmm_config.parameters.k", 8);
  double radius = xml_data.get("fmm_config.parameters.r", 300.0);
  double gps_error = xml_data.get("fmm_config.parameters.gps_error", 50.0);
  double vmax = xml_data.get("fmm_config.parameters.vmax", 80.0);;
  double factor = xml_data.get("fmm_config.parameters.factor", 1.5);;
  return STMATCHConfig{k,radius,gps_error,vmax,factor};
};

STMATCHConfig STMATCHConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data){
  int k = arg_data["candidates"].as<int>();
  double radius = arg_data["radius"].as<double>();
  double gps_error = arg_data["error"].as<double>();
  double vmax = arg_data["vmax"].as<double>();
  double factor = arg_data["factor"].as<double>();
  return STMATCHConfig{k,radius,gps_error,vmax,factor};
};

};
