//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_ARG_CONFIG_HPP_
#define FMM_SRC_CONFIG_ARG_CONFIG_HPP_

#include "config/gps_config.hpp"
#include "config/network_config.hpp"
#include "config/result_config.hpp"

namespace MM{

class ArgConfig{
public:
  ArgConfig(int argc,char **argv){
    data = opts.parse(argc,argv);
  };
  NetworkConfig get_network_config() const;
  GPSConfig get_gps_config() const;
  ResultConfig get_result_config() const;
private:
  bool data_projected = false;
  int step = 0;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level = 5;
  std::string ubodt_file;
  bool help_specified = false;
  ParseResult data;
};
}

#endif //FMM_SRC_CONFIG_ARG_CONFIG_HPP_
