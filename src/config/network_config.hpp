//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_
#define FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_

#include <string>
#include "cxxopts/cxxopts.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace MM{

struct NetworkConfig{
  std::string file;
  std::string id;
  std::string source;
  std::string target;
  bool validate() const;
  std::string to_string() const;
  static NetworkConfig load_from_xml(
    const boost::property_tree::ptree &xml_data);
  static NetworkConfig load_from_arg(
    const cxxopts::ParseResult &arg_data);
};

}

#endif //FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_
