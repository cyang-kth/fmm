#include "config/network_config.hpp"
#include "util/util.hpp"
#include "util/debug.hpp"

void MM::CONFIG::NetworkConfig::print() const{
  SPDLOG_INFO("NetworkConfig");
  SPDLOG_INFO("File name: {} ",file);
  SPDLOG_INFO("ID name: {} ",id);
  SPDLOG_INFO("Source name: {} ",source);
  SPDLOG_INFO("Target name: {} ",target);
};

MM::CONFIG::NetworkConfig MM::CONFIG::NetworkConfig::load_from_xml(
  const boost::property_tree::ptree &xml_data){
  std::string file = xml_data.get<std::string>("config.input.network.file");
  std::string id = xml_data.get("config.input.network.id", "id");
  std::string source = xml_data.get("config.input.network.source","source");
  std::string target = xml_data.get("config.input.network.target","target");
  return MM::CONFIG::NetworkConfig{file, id, source, target};
};

MM::CONFIG::NetworkConfig MM::CONFIG::NetworkConfig::load_from_arg(
  const cxxopts::ParseResult &arg_data){
  std::string file = arg_data["network"].as<std::string>();
  std::string id = arg_data["network_id"].as<std::string>();
  std::string source = arg_data["source"].as<std::string>();
  std::string target = arg_data["target"].as<std::string>();
  return MM::CONFIG::NetworkConfig{file, id, source, target};
};

bool MM::CONFIG::NetworkConfig::validate() const {
  if (!UTIL::file_exists(file)){
    SPDLOG_CRITICAL("Network file not found {}",file);
    return false;
  }
  return true;
}
