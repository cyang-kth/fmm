/**
 * Fast map matching.
 *
 * Definition of input trajectory format
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_GPS_HPP
#define FMM_GPS_HPP

#include "core/geometry.hpp"

#include <vector>

namespace FMM
{

namespace CORE
{

/**
 * %Trajectory class
 *
 * A GPS trajectory represented with id, geometry and timestamps
 */
struct Trajectory
{
    Trajectory(){};
    Trajectory(int id_arg, const LineString &geom_arg)
        : id(id_arg), geom(geom_arg){};
    Trajectory(int id_arg, const LineString &geom_arg,
               const std::vector<double> &timestamps_arg)
        : id(id_arg), geom(geom_arg), timestamps(timestamps_arg){};
    int id;                         /**< Id of the trajectory */
    LineString geom;                /**< Geometry of the trajectory */
    std::vector<double> timestamps; /**< Timestamps of the trajectory */
};

} // namespace CORE

} // namespace FMM
#endif /* FMM_GPS_HPP */
