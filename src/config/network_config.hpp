//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_
#define FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_

#include<string>

namespace MM{

struct NetworkConfig{
  std::string file;
  std::string id;
  std::string source;
  std::string target;
  static NetworkConfig load_from_xml(){

  };
  static NetworkConfig load_from_arg(){

  };
};

}

#endif //FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_
