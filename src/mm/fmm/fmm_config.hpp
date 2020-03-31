/**
 * Content
 * Configuration Class for FMM
 *
 * @author: Can Yang
 * @version: 2019.03.27
 */
#ifndef MM_FMM_CONFIG_HPP
#define MM_FMM_CONFIG_HPP

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "cxxopts/cxxopts.hpp"

namespace MM
{

struct FMMConfig{
  int k;
  double radius;
  double gps_error;
  std::string to_string() const;
  static FMMConfig load_from_xml(
    const boost::property_tree::ptree &xml_data);
  static FMMConfig load_from_arg(
    const cxxopts::ParseResult &arg_data);
};

} // MM
#endif //MM_FMM_CONFIG_HPP
