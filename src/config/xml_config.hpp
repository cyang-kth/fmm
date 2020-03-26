//
// Created by Can Yang on 2020/3/26.
//

#ifndef FMM_SRC_CONFIG_XML_CONFIG_HPP_
#define FMM_SRC_CONFIG_XML_CONFIG_HPP_

namespace MM{
class XMLConfig{
public:
  XMLConfig(const std::string &filename){
    boost::property_tree::read_xml(filename, data);
  };
  boost::property_tree::ptree data;
};
}

#endif //FMM_SRC_CONFIG_XML_CONFIG_HPP_
