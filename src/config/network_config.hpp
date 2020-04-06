/**
 * Fast map matching.
 *
 * Network configuration class
 *
 * @author Can Yang
 */

#ifndef FMM_NETWORK_CONFIG_HPP_
#define FMM_NETWORK_CONFIG_HPP_

#include <string>
#include "cxxopts/cxxopts.hpp"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace FMM{

namespace CONFIG {
/**
 *  Network configuration class for reading network from a file.
 */
struct NetworkConfig{
  std::string file; /**< filename */
  std::string id; /**< id field/column name */
  std::string source; /**< source field/column name */
  std::string target; /**< target field/column name */
  /**
   * Validate the GPS configuration for file existence.
   * @return if file exists returns true, otherwise return false
   */
  bool validate() const;
  /**
   * Print informaiton
   */
  void print() const;
  /**
   * Load NetworkConfig from xml data
   * @param  xml_data [description]
   * @return          [description]
   */
  static NetworkConfig load_from_xml(
    const boost::property_tree::ptree &xml_data);
  /**
   * Load NetworkConfig from argument parsed data
   * @param  xml_data [description]
   * @return          [description]
   */
  static NetworkConfig load_from_arg(
    const cxxopts::ParseResult &arg_data);
};

} // CONFIG

} // FMM

#endif //FMM_NETWORK_CONFIG_HPP_
