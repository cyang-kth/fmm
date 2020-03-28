//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_
#define FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_

#include<string>

namespace MM{

struct NetworkConfig{
  std::string network_file;
  std::string network_id;
  std::string network_source;
  std::string network_target;
};

}

#endif //FMM_SRC_CONFIG_NETWORK_CONFIG_HPP_
