//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_XML_CONFIG_HPP_
#define FMM_SRC_CONFIG_XML_CONFIG_HPP_

namespace MM{
class XMLConfig{
public:
  XMLConfig(const std::string &filename){
    boost::property_tree::read_xml(filename, data);
  };
  NetworkConfig get_network_config() const;
  GPSConfig get_gps_config() const;
  ResultConfig get_result_config() const;
private:
  boost::property_tree::ptree data;
  bool data_projected = false;
  int step = 0;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level = 5;
  bool help_specified = false;
};
}

#endif //FMM_SRC_CONFIG_XML_CONFIG_HPP_
