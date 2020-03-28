#include "config/network_config.hpp"

namespace MM {

NetworkConfig NetworkConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data){
  std::string file = xml_data.get<std::string>("fmm_config.input.network.file");
  std::string id = xml_data.get("fmm_config.input.network.id", "id");
  std::string source = xml_data.get("fmm_config.input.network.source","source");
  std::string target = xml_data.get("fmm_config.input.network.target","target");
  return NetworkConfig{file,id,source,target};
};

NetworkConfig NetworkConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data){
  std::string file = arg_data["network"].as<std::string>();
  std::string id = arg_data["network_id"].as<std::string>();
  std::string source = arg_data["source"].as<std::string>();
  std::string target = arg_data["target"].as<std::string>();
  return NetworkConfig{file,id,source,target};
};
}
