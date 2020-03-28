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
private:
  bool data_projected = false;
  int step = 0;
  // 0-trace,1-debug,2-info,3-warn,4-err,5-critical,6-off
  int log_level = 5;
  std::string ubodt_file;
  bool help_specified = false;
  // Whether export the nodes visited or not.
  bool verbose = false;
};
}

#endif //FMM_SRC_CONFIG_ARG_CONFIG_HPP_
