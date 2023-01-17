/**
 * Fast map matching.
 *
 * Configuration of reading gps data
 *
 * @author Can Yang
 */

#ifndef FMM_SRC_CONFIG_GPS_CONFIG_HPP_
#define FMM_SRC_CONFIG_GPS_CONFIG_HPP_

#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "cxxopts/cxxopts.hpp"

namespace FMM
{
/**
 * Classes related with configuration
 */
namespace CONFIG
{
/**
 *  GPS configuration class for reading data from a file.
 */
struct GPSConfig
{
    GPSConfig(const std::string &file_arg = "",
              const std::string &id_arg = "id",
              const std::string &geom_arg = "geom",
              const std::string &x_arg = "x", const std::string &y_arg = "y",
              const std::string &timestamp_arg = "timestamp",
              bool gps_point_arg = false)
        : file(file_arg), id(id_arg), geom(geom_arg), x(x_arg), y(y_arg),
          timestamp(timestamp_arg), gps_point(gps_point_arg){};
    std::string file;      /**< filename */
    std::string id;        /**< id field/column name */
    std::string geom;      /**< geometry field/column name */
    std::string x;         /**< x field/column name */
    std::string y;         /**< y field/column name */
    std::string timestamp; /**< timestamp field/column name */
    bool gps_point;        /**< gps point stored or not */
    /**
     * Validate the GPS configuration for file existence, parameter validation
     * @return true if validate success, otherwise false returned
     */
    bool validate() const;
    /**
     * Print members of the GPS configuration.
     */
    void print() const;
    /**
     * Find the GPS format.
     *
     * @return 0 for GDAL trajectory file, 1 for CSV trajectory file
     * and 2 for CSV point file, otherwise -1 is returned for unknown
     * format.
     */
    int get_gps_format() const;

    std::string to_string() const;
    /**
     * Load GPSConfig from XML data.
     *
     * @param xml_data
     * @return
     */
    static GPSConfig load_from_xml(const boost::property_tree::ptree &xml_data);
    /**
     * Load GPSConfig from argument parsed data.
     *
     * @param arg_data
     * @return
     */
    static GPSConfig load_from_arg(const cxxopts::ParseResult &arg_data);
    /**
     * Register arguments to an option object
     */
    static void register_arg(cxxopts::Options &options);
    /**
     * Register help information to a string stream
     */
    static void register_help(std::ostringstream &oss);
};

} // namespace CONFIG

} // namespace FMM

#endif // FMM_SRC_CONFIG_GPS_CONFIG_HPP_
