/**
 * Fast map matching.
 *
 * Python API data type
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

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
  int index; /**< Index of the candidate, point index */
  int edge_id; /**< Edge id matched */
  int source; /**< Edge source node ID */
  int target; /**< Edge target node ID */
  double error; /**< Error of matching */
  double offset; /**< Matched point distance to start node of edge */
  double length; /**< Length of edge matched */
  double ep; /**< emission probability */
  double tp; /**< transition proability from previous matched candidate */
  double spdist; /**< shortest path distance from previous matched candidate */
};

/**
 * POD Match result type used in Python API
 */
struct PyMatchResult {
  int id; /**< id of a trajectory */
  MM::O_Path opath; /**< Edge ID matched for each point of the trajectory  */
  MM::C_Path cpath; /**< Edge ID traversed by the matched path */
  std::vector<PyCandidate> candidates; /**< Candidate matched to each point */
  std::vector<int> indices; /**< index of matched edge in the cpath */
  CORE::LineString mgeom; /**< Geometry of the matched path */
  CORE::LineString pgeom; /**< Point position matched for each GPS point */
};
}; // PYTHON
}; // FMM

#endif //FMM_SRC_PYTHON_PYFMM_HPP_
