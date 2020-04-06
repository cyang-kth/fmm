//
// Created by Can Yang on 2020/4/3.
//

#ifndef FMM_PYFMM_HPP_
#define FMM_PYFMM_HPP_

#include "mm/mm_type.hpp"

namespace FMM{
/**
 * Data type for Python API
 */
namespace PYTHON{
/**
 * POD Candidate data type used in Python API
 */
struct PyCandidate{
  int index;       // point index in trajectory
  int edge_id;       // edge id
  int source;
  int target;
  double error;
  double offset;
  double length;
  double ep;
  double tp;
  double spdist;
};

/**
 * POD Match result type used in Python API
 */
struct PyMatchResult {
  int id;
  MM::O_Path opath;
  MM::C_Path cpath;
  std::vector<PyCandidate> candidates;
  std::vector<int> indices; // index of opath edge in cpath
  CORE::LineString mgeom;
  CORE::LineString pgeom;
};
}; // PYTHON
}; // FMM

#endif //FMM_SRC_PYTHON_PYFMM_HPP_
