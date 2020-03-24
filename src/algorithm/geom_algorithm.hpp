/**
 * Content
 * Functions for linear referencing whch finds the closest point on a
 * a polyline given an input point.
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_GEOM_ALGORITHM_HPP
#define MM_GEOM_ALGORITHM_HPP

#include "core/geometry.hpp"

namespace MM
{
namespace ALGORITHM {

std::vector<double> cal_eu_dist(const LineString &trajectory);

void append_segs_to_line(LineString *line, const LineString &segs,
                         int offset=0);

LineString reverse_geometry(LineString &rhs);

// Split a line at every delta length point
std::vector<LineString> split_line(const LineString &line,double delta);

LineString interpolate_line_distances(const LineString &line,
                                      const std::vector<double> &distances);

LineString interpolate_line_distance(const LineString &line,
                                     double distance);

LineString interpolate_line_kpoints(const LineString &line, int k);

/**
 * Compute the boundary of an LineString and returns the result in
 * the passed x1,y1,x2,y2 variables.
 *
 * @param linestring: input, which is a pointer to a
 * linestring object
 * @param x1,y1,x2,y2: the coordinates of the boundary
 */
void boundingbox_geometry(const LineString &linestring,
                          double *x1,double *y1,double *x2,double *y2);

std::vector<double> calc_length_to_end_vec(const LineString &geom);

/**
 * Project a point p=(x,y); to a directed segment of (x1,y1);->(x2,y2);
 * Let p' denote the projected point, the following
 * information is stored in the passed variables
 *
 * @param dist   the distance from p to p'
 * @param offset the distance from the start of the segement (x1,y1);
 *  to p'
 */
void closest_point_on_segment(double x,double y,double x1,double y1,
                              double x2,double y2,double *dist,double *offset);

void closest_point_on_segment(double x, double y, double x1, double y1,
                              double x2, double y2, double *dist,
                              double *offset, double *closest_x,
                              double *closest_y);


/**
 * A linear referencing function
 * Given a point and a polyline, return the projected distance (p to p');
 * and offset distance (the distance along the polyline from its start
 * to the projected point p'); in the passed variables
 * @param x,y         Coordinates of the point to be queried
 * @param linestring    input linestring
 * @param result_dist   output projected distance
 * @param result_offset output offset distance from the start of the
 * polyline
 */
void linear_referencing(double px, double py,
                        const LineString &linestring, double *result_dist,
                        double *result_offset);

/**
 *  Linear referencing function with projected point returned
 * */
void linear_referencing(double px, double py, const LineString &linestring,
                        double *result_dist, double *result_offset,
                        double *proj_x,double *proj_y);

/**
 * Locate the point on a linestring according to the input of offset
 * The two pointer's target value will be updated.
 */
void locate_point_by_offset(const LineString &linestring, double offset,
                            double *x, double *y); // calculate_offset_point

LineString cutoffseg_unique(double offset1, double offset2,
                            const LineString &linestring);

/**
 * added by Diao 18.01.17
 * modified by Can 18.01.19
 * modified by Can 18.03.14
 *
 * @param   offset        input offset(from start node);
 * @param   linestring    input linestring
 * @param   mode          input mode, 0 represent cutoff from start node,
 *                        namely export the part p->end , 1 from endnode
 *                        export the part of start-> p
 * @return  cutoffline    output cutoff linstring, the caller
 *                        should take care of freeing the memory
 */
LineString cutoffseg(double offset, const LineString &linestring, int mode);

} // ALGORITHM
} // MM
#endif /* MM_ALGORITHM_HPP */
