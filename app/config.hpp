/**
 * Content
 * Configuration Class defined for the two application
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */
#ifndef MM_CONFIG_HPP
#define MM_CONFIG_HPP
// Boost propertytree library
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <set>
#include <sys/stat.h> // file exist test
#include <exception>
#include <iomanip>

namespace MM
{

bool fileExists(std::string &filename)
{
    struct stat buf;
    if (stat(filename.c_str(), &buf) != -1)
    {
        return true;
    }
    return false;
};
// Check extension of the file, 0 for CSV and 1 for Binary
int get_file_extension(std::string &fn) {
    std::string fn_extension = fn.substr(fn.find_last_of(".") + 1);
    if (fn_extension == "csv" || fn_extension == "txt") {
        return 0;
    } else if (fn_extension == "bin" || fn_extension == "binary") {
        return 1;
    }
    return 2;
};

/**
 * Configuration class for map matching
 */
class FMM_Config
{
public:
    /**
     * FILETYPE 0 for ini and 1 for XML
     */
    FMM_Config(const std::string &file)
    {

        // Create empty property tree object
        boost::property_tree::ptree tree;
        boost::property_tree::read_xml(file, tree);
        // Parse the XML into the property tree.
        // Without default value, the throwing version of get to find attribute.
        // If the path cannot be resolved, an exception is thrown.

        // UBODT
        ubodt_file = tree.get<std::string>("fmm_config.input.ubodt.file");
        multiplier = tree.get("fmm_config.input.ubodt.multiplier", 37); // multiplier=30000
        nhash = tree.get("fmm_config.input.ubodt.nhash", 127); // 5178049
        delta = tree.get("fmm_config.input.ubodt.delta", 5000);
        binary_flag = get_file_extension(ubodt_file);

        // Network
        network_file = tree.get<std::string>("fmm_config.input.network.file");
        network_id = tree.get("fmm_config.input.network.id", "id");
        network_source = tree.get("fmm_config.input.network.source", "source");
        network_target = tree.get("fmm_config.input.network.target", "target");

        // GPS
        gps_file = tree.get<std::string>("fmm_config.input.gps.file");
        gps_id = tree.get("fmm_config.input.gps.id", "id");

        // Other parameters
        k = tree.get("fmm_config.parameters.k", 8);
        radius = tree.get("fmm_config.parameters.r", 300.0);

        // HMM
        gps_error = tree.get("fmm_config.parameters.gps_error", 50.0);
        penalty_factor = tree.get("fmm_config.parameters.pf", 0.0);

        // Output
        result_file = tree.get<std::string>("fmm_config.output.file");
        mode = tree.get("fmm_config.output.mode", 0);
    };
    void print()
    {
        std::cout << "------------------------------------------" << '\n';
        std::cout << "Configuration parameters for map matching application: " << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Network_file: " << network_file << '\n';;
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Network id: " << network_id << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Network source: " << network_source << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Network target: " << network_target << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "ubodt_file: " << ubodt_file << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "multiplier: " << multiplier << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "nhash: " << nhash << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "delta: " << delta << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "ubodt format(1 binary, 0 csv): " << binary_flag << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "gps_file: " << gps_file << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "gps_id: " << gps_id << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "k: " << k << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "radius: " << radius << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "gps_error: " << gps_error << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "penalty_factor: " << penalty_factor << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "result_file:" << result_file << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "geometry mode: " << mode << "(0 no geometry, 1 wkb, 2 wkt)" << '\n';
        std::cout << "------------------------------------------" << '\n';
    };
    bool validate_mm()
    {
        std::cout << "Validating configuration for map match application:" << '\n';
        if (!fileExists(gps_file))
        {
            std::cout << std::setw(12) << "" << "Error, GPS_file not found. Program stop." << '\n';
            return false;
        };
        if (!fileExists(network_file))
        {
            std::cout << std::setw(12) << "" << "Error, Network file not found. Program stop." << '\n';
            return false;
        };
        if (!fileExists(ubodt_file))
        {
            std::cout << std::setw(12) << "" << "Error, UBODT file not found. Program stop." << '\n';
            return false;
        };
        if (binary_flag==2){
            std::cout << std::setw(12) << "" << "Error, UBODT file extension not recognized, which should be csv or binary.  Program stop." << '\n';
            return false;
        }
        if (fileExists(result_file))
        {
            std::cout << std::setw(4) << "" << "Warning, overwrite existing result file." << result_file << '\n';
        };
        if (gps_error <= 0 || multiplier <= 0 || nhash <= 0 || radius <= 0 || k <= 0)
        {
            std::cout << std::setw(12) << "" << "Error, Algorithm parameters invalid." << '\n';
            return false;
        }
        if (mode > 3) {
            std::cout << std::setw(12) << "" << "Error, Unrecognized map matching mode: "<< mode << '\n';
            return false; 
        } 
        // Check the definition of parameters search radius and gps error
        if (radius / gps_error > 10) {
            std::cout << std::setw(12) << "" << "Error, the gps error " << gps_error
                      << " is too small compared with search radius " << radius << '\n';
            std::cout << std::setw(12) << "It may cause underflow, try to increase gps error or descrease search radius" << '\n';
            return false;
        }
        std::cout << "Validating success." << '\n';
        return true;
    };
    /* Input files */
    // Network file
    std::string network_file;
    std::string network_id;
    std::string network_source;
    std::string network_target;

    // UBODT configurations
    std::string ubodt_file;
    int multiplier;
    int nhash;
    double delta;
    int binary_flag;

    // GPS file
    std::string gps_file;
    std::string gps_id;
    // Result file
    std::string result_file;
    /*
        0 for no geometry construction, only optimal path and complete
        path will be outputed
        1 for wkb geometry output
        2 for wkt geometry output
    */
    int mode;
    // Parameters
    double gps_error;
    // Used by hashtable in UBODT

    // Used by Rtree search
    int k;
    double radius;

    // PF for reversed movement
    double penalty_factor;
}; // FMM_Config


/**
 * Configuration class for UBODT
 */
class UBODT_Config
{
public:
    /**
     * FILETYPE 0 for ini and 1 for XML
     */
    UBODT_Config(const std::string &file)
    {

        // Create empty property tree object
        boost::property_tree::ptree tree;
        std::cout << "Read configuration from xml file: " << file << '\n';
        boost::property_tree::read_xml(file, tree);
        // Parse the XML into the property tree.
        // Without default value, the throwing version of get to find attribute.
        // If the path cannot be resolved, an exception is thrown.

        // UBODT configuration
        delta = tree.get("ubodt_config.parameters.delta", 5000);

        // Network
        network_file = tree.get<std::string>("ubodt_config.input.network.file");
        network_id = tree.get("ubodt_config.input.network.id", "id");
        network_source = tree.get("ubodt_config.input.network.source", "source");
        network_target = tree.get("ubodt_config.input.network.target", "target");
        // int temp = tree.get("ubodt_config.input.network.nid_index",0);
        // nid_index= temp>0;
        // Output
        result_file = tree.get<std::string>("ubodt_config.output.file");
        binary_flag = get_file_extension(result_file);
        // binary_flag = tree.get("ubodt_config.output.binary", 1);
    };
    void print()
    {
        std::cout << "------------------------------------------" << '\n';
        std::cout << "Configuration parameters for UBODT construction: " << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Network_file: " << network_file << '\n';;
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Network id: " << network_id << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Network source: " << network_source << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Network target: " << network_target << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "delta: " << delta << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Output file:" << result_file << '\n';
        std::cout << std::left << std::setw(4) << "" << std::setw(20) << "Output format(1 binary, 0 csv): " << binary_flag << '\n';
        std::cout << "------------------------------------------" << '\n';
    };
    bool validate()
    {
        std::cout << "Validating configuration for UBODT construction:" << '\n';
        if (!fileExists(network_file))
        {
            std::cout << std::setw(12) << "" << "Error,Network file not found" << '\n';
            return false;
        }
        if (fileExists(result_file))
        {
            std::cout << std::setw(4) << "" << "Warning, overwrite existing result file." << result_file << '\n';
        }
        if (binary_flag==2){
            std::cout << std::setw(12) << "" << "Error, UBODT file extension not recognized, which should be csv or binary.  Program stop." << '\n';
            return false;
        }
        if (delta <= 0)
        {
            std::cout << std::setw(12) << "" << "Error,Delta value should be positive." << '\n';
            return false;
        }
        std::cout << "Validating success." << '\n';
        return true;
    };
    std::string network_file;
    std::string network_id;
    std::string network_source;
    std::string network_target;
    int binary_flag;
    double delta;
    std::string result_file;
}; // UBODT_Config

} // MM
#endif //MM_CONFIG_HPP
