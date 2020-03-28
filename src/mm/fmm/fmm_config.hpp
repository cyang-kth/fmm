/**
 * Content
 * Configuration Class for FMM
 *
 * @author: Can Yang
 * @version: 2019.03.27
 */
#ifndef MM_FMM_CONFIG_HPP
#define MM_FMM_CONFIG_HPP

namespace MM
{

struct FMMConfig{
  int k;
  double radius; //meter
  double gps_error; //meter
}

} // MM
#endif //MM_FMM_CONFIG_HPP
