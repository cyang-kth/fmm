//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_ARG_CONFIG_HPP_
#define FMM_SRC_CONFIG_ARG_CONFIG_HPP_

namespace MM{
class ArgConfig{
 public:
  ArgConfig(int argc,char **argv){
    data = opts.parse(argc,argv);
  };
  ParseResult data;
};
}

#endif //FMM_SRC_CONFIG_ARG_CONFIG_HPP_
