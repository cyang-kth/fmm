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
};

}

#endif //FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_
