/**
 * Content
 * Configuration Class for FMM
 *
 * @author: Can Yang
 * @version: 2019.03.27
 */
#ifndef MM_STMATCH_CONFIG_HPP
#define MM_STMATCH_CONFIG_HPP

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "cxxopts/cxxopts.hpp"

namespace MM
{

struct STMATCHConfig {
  int k;
  double radius;
  double gps_error;
  // maximum speed of the vehicle
  double vmax;
  // factor multiplied to vmax*deltaT to limit the search of shortest path
  double factor;
  std::string to_string() const;
  static STMATCHConfig load_from_xml(
    const boost::property_tree::ptree &xml_data);
  static STMATCHConfig load_from_arg(
    const cxxopts::ParseResult &arg_data);
};

} // MM
#endif //MM_STMATCH_CONFIG_HPP
