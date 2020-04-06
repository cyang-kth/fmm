/**
 * Fast map matching.
 * 
 * Algorithms for geoprocessing.
 *
 * Updated by Can Yang 2020-04-05
 * Updated by Diaolin 18.01.17
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef FMM_GEOM_ALGORITHM_HPP
#define FMM_GEOM_ALGORITHM_HPP

#include "core/geometry.hpp"

namespace FMM {
/**
 * Algorithms for geoprocessing.
 */
namespace ALGORITHM {
/**
 * Calculate segment length of a trajectory
 * @param trajectory a trajectory as input
 * @return A vector of double values with size of N-1.
 * N is the number of points in trajectory.
 */
std::vector<double> cal_eu_dist(const FMM::CORE::LineString &trajectory);

/**
 * Concatenate a linestring segs to a linestring line, used in the
 * function network.complete_path_to_geometry
 *
 * @param line: linestring which will be updated
 * @param segs: segs that will be appended to line
 * @param offset: the number of points skipped in segs.
 */
void append_segs_to_line(FMM::CORE::LineString *line,
                         const FMM::CORE::LineString &segs,
                         int offset = 0);

/**
 * Create a new linestring as the reverse of an input linestring
 *
 * @param rhs the input linestring
 * @return A linestring containing points in the reverse order.
 */
FMM::CORE::LineString reverse_geometry(const FMM::CORE::LineString &rhs);

/**
 * Interpolate a linestring at a fixed distance threshold and return the result
 * in as a vector of linestring
 *
 * @param line input line
 * @param delta the distance threshold to split the line
 * @return A vector of linestring
 */
std::vector<FMM::CORE::LineString> split_line(
    const FMM::CORE::LineString &line, double delta);

/**
 * Interpolate a linestring according to a vector of distances to
 * the start point of a line
 * @param line
 * @param distances a vector of distance values to the start point
 * @return a linestring containing the interpolated points
 */
FMM::CORE::LineString interpolate_line_distances(
    const FMM::CORE::LineString &line, const std::vector<double> &distances);

/**
 * Interpolate a linestring according to a distance step value
 * @param line the input linestring
 * @param distance the distance step value
 * @return a linestring containing the interpolated points
 */
FMM::CORE::LineString interpolate_line_distance(
    const FMM::CORE::LineString &line, double distance);

/**
 * Interpolate k points in a linestring with equal distance
 * @param line the input linestring
 * @param k interpolate k points in equal distances
 * @return a linestring containing the interpolated points
 */
FMM::CORE::LineString interpolate_line_kpoints(
    const FMM::CORE::LineString &line, int k);

/**
 * Compute the boundary of an FMM::CORE::LineString and returns the result in
 * the passed x1,y1,x2,y2 variables.
 *
 * @param linestring input line
 * @param x1 the bottom left point x coordinate
 * @param y1 the bottom left point y coordinate
 * @param x2 the top right point x coordinate
 * @param y2 the top right point y coordinate
 */
void boundingbox_geometry(const FMM::CORE::LineString &linestring,
                          double *x1, double *y1, double *x2, double *y2);

/**
 * Calculate the distance from each point in a linestring to the end point
 * of a linestring
 * @param geom input linestring
 * @return a vector of distance values
 */
std::vector<double> calc_length_to_end_vec(const FMM::CORE::LineString &geom);

/**
 * Calculate the closest point p' on a segment p1 (x1,y1) p2 (x2,y2) to a
 * specific point p (x,y)
 *
 * @param x
 * @param y
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param dist the distance from p to p'
 * @param offset the distance from p1 to p'
 */
void closest_point_on_segment(double x, double y, double x1, double y1,
                              double x2, double y2,
                              double *dist, double *offset);

/**
 *
 * Calculate the closest point p' on a segment p1 (x1,y1) p2 (x2,y2) to a
 * specific point p (x,y)
 *
 * @param x
 * @param y
 * @param x1
 * @param y1
 * @param x2
 * @param y2
 * @param dist the distance from p to p'
 * @param offset the distance from p1 to p'
 * @param closest_x the x coordinate of p'
 * @param closest_y the y coordinate of p'
 */
void closest_point_on_segment(double x, double y, double x1, double y1,
                              double x2, double y2, double *dist,
                              double *offset, double *closest_x,
                              double *closest_y);

/**
 * A linear referencing function
 *
 * Given a point p (px,py) and a polyline, find the closest point p' on line
 * and return the projected distance (p to p') and offset distance
 * (the distance along the polyline from its start to ')
 * @param px x coordinate of p
 * @param py y coordinate of p
 * @param linestring input line
 * @param result_dist the projected distance, the pointer will be updated
 * @param result_offset the offset distance, the pointer will be updated
 */
void linear_referencing(double px, double py,
                        const FMM::CORE::LineString &linestring,
                        double *result_dist,
                        double *result_offset);

/**
 * A linear referencing function
 *
 * Given a point p (px,py) and a polyline, find the closest point p' on line
 * and return the projected distance (p to p') and offset distance
 * (the distance along the polyline from its start to ')
 * @param px x coordinate of p
 * @param py y coordinate of p
 * @param linestring input line
 * @param result_dist the projected distance
 * @param result_offset the offset distance
 * @param proj_x x coordinate of p'
 * @param proj_y y coordinate of p'
 */
void linear_referencing(double px, double py,
                        const FMM::CORE::LineString &linestring,
                        double *result_dist, double *result_offset,
                        double *proj_x, double *proj_y);

/**
 * Locate the point on a linestring according to the input of offset
 * The two pointer's target value will be updated.
 */
/**
 * Locate a point p on a linestring according to an offset value
 * @param linestring
 * @param offset the distance from p to start point of linestring
 * @param x the x coordinate of p
 * @param y the y coordinate of p
 */
void locate_point_by_offset(const FMM::CORE::LineString &linestring,
                            double offset, double *x, double *y);

/**
 * Cut a linestring at two offset values
 * @param linestring input line
 * @param offset1 starting offset, distance to the start point of linestring
 * @param offset2 ending offset, distance to the start point of linestring
 * @return a linestring containing only the part covering starting offset to
 * ending offset
 */
FMM::CORE::LineString cutoffseg_unique(
    const FMM::CORE::LineString &linestring, double offset1, double offset2);

/**
 * Added by Diao 18.01.17
 * modified by Can 18.01.19
 * modified by Can 18.03.14
 *
 * Cut a linestring at an offset value according to a mode value.
 * If mode is 0, cutting from offset to the end of linestring,
 * otherwise cutting from the starting point to the offset value
 * @param linestring input linestring
 * @param offset offset value, distance to the start point of linestring
 * @param mode cutting mode value
 * @return a linestring that is cut from input linestring
 */
FMM::CORE::LineString cutoffseg(
    const FMM::CORE::LineString &linestring, double offset, int mode);

} // ALGORITHM
} // FMM
#endif /* FMM_ALGORITHM_HPP */
