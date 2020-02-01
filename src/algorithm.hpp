/**
 * Content
 * Functions for linear referencing whch finds the closest point on a
 * a polyline given an input point.
 *
 * @author: Can Yang
 * @version: 2020.01.23 - LineString pointer removed.
 * @version: 2017.11.11
 */

#ifndef MM_ALGORITHM_HPP
#define MM_ALGORITHM_HPP
#include <cmath>
#include "types.hpp"
#include "debug.h"
#include "util.hpp"
namespace MM
{
namespace ALGORITHM {

/**
 * Compute the boundary of an LineString and returns the result in
 * the passed x1,y1,x2,y2 variables.
 *
 * @param linestring: line geometry
 * @param x1,y1,x2,y2: the coordinates of the boundary
 */
void boundingbox_geometry(const LineString &linestring,
                          double *x1,double *y1,double *x2,double *y2)
{
  int Npoints = linestring.getNumPoints();
  *x1 = DBL_MAX;
  *y1 = DBL_MAX;
  *x2 = DBL_MIN;
  *y2 = DBL_MIN;
  double x,y;
  for(int i=0; i<Npoints; ++i)
  {
    x = linestring.getX(i);
    y = linestring.getY(i);
    if (x<*x1) *x1 = x;
    if (y<*y1) *y1 = y;
    if (x>*x2) *x2 = x;
    if (y>*y2) *y2 = y;
  };
}; // boundingbox_geometry

/**
 * Project a point p=(x,y) to a directed segment of (x1,y1)->(x2,y2)
 * Let p' denote the projected point, the following
 * information is stored in the passed variables
 *
 * @param dist   the distance from p to p'
 * @param offset the distance from the start of the segement (x1,y1)
 *  to p'
 */
void closest_point_on_segment(double x,double y,double x1,double y1,
                              double x2,double y2,double *dist,double *offset)
{
  double L2 = (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
  if (L2 == 0.0)
  {
    *dist=std::sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
    *offset=0.0;
    return;
  }
  double x1_x = x-x1;
  double y1_y = y-y1;
  double x1_x2 = x2-x1;
  double y1_y2 = y2-y1;
  double ratio = (x1_x*x1_x2+y1_y*y1_y2)/L2;
  ratio=(ratio>1) ? 1 : ratio;
  ratio=(ratio<0) ? 0 : ratio;
  double prj_x = x1+ ratio*(x1_x2);
  double prj_y = y1+ ratio*(y1_y2);
  *offset = std::sqrt((prj_x-x1)*(prj_x-x1)+(prj_y-y1)*(prj_y-y1));
  *dist = std::sqrt((prj_x-x)*(prj_x-x)+(prj_y-y)*(prj_y-y));
}; // closest_point_on_segment

void closest_point_on_segment(double x, double y, double x1, double y1,
                              double x2, double y2, double *dist,
                              double *offset, double *closest_x,
                              double *closest_y)
{
  double L2 = (x2-x1)*(x2-x1)+(y2-y1)*(y2-y1);
  if (L2 == 0.0)
  {
    *dist=std::sqrt((x-x1)*(x-x1)+(y-y1)*(y-y1));
    *offset=0.0;
    *closest_x=x1;
    *closest_y=y1;
    return;
  }
  double x1_x = x-x1;
  double y1_y = y-y1;
  double x1_x2 = x2-x1;
  double y1_y2 = y2-y1;
  double ratio = (x1_x*x1_x2+y1_y*y1_y2)/L2;
  ratio=(ratio>1) ? 1 : ratio;
  ratio=(ratio<0) ? 0 : ratio;
  double prj_x = x1+ ratio*(x1_x2);
  double prj_y = y1+ ratio*(y1_y2);
  *offset = std::sqrt((prj_x-x1)*(prj_x-x1)+(prj_y-y1)*(prj_y-y1));
  *dist = std::sqrt((prj_x-x)*(prj_x-x)+(prj_y-y)*(prj_y-y));
  *closest_x = prj_x;
  *closest_y = prj_y;
}; // closest_point_on_segment

/**
 * A linear referencing function
 * Given a point and a polyline, return the projected distance (p to p')
 * and offset distance (the distance along the polyline from its start
 * to the projected point p') in the passed variables
 * @param x,y         Coordinates of the point to be queried
 * @param linestring    input linestring
 * @param result_dist   output projected distance
 * @param result_offset output offset distance from the start of the
 * polyline
 */
void linear_referencing(double px, double py,
                        const LineString &linestring, double *result_dist,
                        double *result_offset)
{
  int Npoints = linestring.getNumPoints();
  double min_dist=DBL_MAX;
  double final_offset=DBL_MAX;
  double length_parsed=0;
  int i=0;
  // Iterating to check p(i) == p(i+2)
  // int seg_idx=0;
  while(i<Npoints-1)
  {
    double x1 = linestring.getX(i);
    double y1 = linestring.getY(i);
    double x2 = linestring.getX(i+1);
    double y2 = linestring.getY(i+1);
    double temp_min_dist;
    double temp_min_offset;
    closest_point_on_segment(px,py,x1,y1,x2,y2,
                             &temp_min_dist,&temp_min_offset);
    if (temp_min_dist<min_dist)
    {
      min_dist=temp_min_dist;
      final_offset = length_parsed+temp_min_offset;
    }
    length_parsed+=std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    ++i;
  };
  *result_dist=min_dist;
  *result_offset=final_offset;
};  // linear_referencing

/**
 *  Linear referencing function with projected point returned
 * */
void linear_referencing(double px, double py, const LineString &linestring,
                        double *result_dist, double *result_offset,
                        double *proj_x,double *proj_y)
{
  int Npoints = linestring.getNumPoints();
  double min_dist=DBL_MAX;
  double temp_x=0, temp_y=0;
  double final_offset=DBL_MAX;
  double length_parsed=0;
  int i=0;
  // Iterating to check p(i) == p(i+2)
  // int seg_idx=0;
  while(i<Npoints-1)
  {
    double x1 = linestring.getX(i);
    double y1 = linestring.getY(i);
    double x2 = linestring.getX(i+1);
    double y2 = linestring.getY(i+1);
    double temp_min_dist;
    double temp_min_offset;
    closest_point_on_segment(px,py,x1,y1,x2,y2,
                             &temp_min_dist,&temp_min_offset,&temp_x,&temp_y);
    if (temp_min_dist<min_dist)
    {
      min_dist=temp_min_dist;
      final_offset = length_parsed+temp_min_offset;
      *proj_x = temp_x;
      *proj_y = temp_y;
    }
    length_parsed+=std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    ++i;
  };
  *result_dist=min_dist;
  *result_offset=final_offset;
};  // linear_referencing


/**
 * added by Diao 18.01.17
 * modified by Can 18.01.19
 * @param   offset1        start offset(from start node)
 * @param   offset2        end offset(from start node)
 * @param   linestring    input linestring
 * @return  cutoffline    output cut linstring
 */
LineString cutoffseg_unique(double offset1, double offset2,
                            const LineString &linestring)
{
  LineString cutoffline;
  int Npoints = linestring.getNumPoints();
  if (Npoints==2) {
    // A single segment
    double x1 = linestring.getX(0);
    double y1 = linestring.getY(0);
    double x2 = linestring.getX(1);
    double y2 = linestring.getY(1);
    double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    double ratio1 = offset1/L;
    double new_x1 = x1+ratio1*(x2-x1);
    double new_y1 = y1+ratio1*(y2-y1);
    double ratio2 = offset2/L;
    double new_x2 = x1+ratio2*(x2-x1);
    double new_y2 = y1+ratio2*(y2-y1);
    cutoffline.addPoint(new_x1, new_y1);
    cutoffline.addPoint(new_x2, new_y2);
  } else {
    // Multiple segments
    double L = 0;
    int i = 0;
    while(i<Npoints-1)
    {
      double x1 = linestring.getX(i);
      double y1 = linestring.getY(i);
      double x2 = linestring.getX(i+1);
      double y2 = linestring.getY(i+1);
      double deltaL = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
      // If L <= offset1 <= L + deltaL
      if (L< offset1 && offset1<L+deltaL) {
        double ratio1 = (offset1-L)/deltaL;
        double new_x1 = x1+ratio1*(x2-x1);
        double new_y1 = y1+ratio1*(y2-y1);
        cutoffline.addPoint(new_x1, new_y1);
      }
      // If offset1 < L < offset2
      if (offset1<L && L< offset2) {
        cutoffline.addPoint(x1,y1);
      }
      // If L <= offset2 <= L + deltaL
      if (L< offset2 && offset2<L+deltaL) {
        double ratio2 = (offset2-L)/deltaL;
        double new_x2 = x1+ratio2*(x2-x1);
        double new_y2 = y1+ratio2*(y2-y1);
        cutoffline.addPoint(new_x2, new_y2);
      }
      L = L + deltaL;
      ++i;
    };
  }
  return cutoffline;
}; //cutoffseg_twoparameters


/**
 * Locate the point on a linestring according to the input of offset
 * The two pointer's target value will be updated.
 */
void locate_point_by_offset(const LineString &linestring, double offset,
                            double *x, double *y){
  int Npoints = linestring.getNumPoints();
  if (offset<=0.0) {
    *x = linestring.getX(0);
    *y = linestring.getY(0);
    return;
  }
  double L_processed=0;
  int i = 0;
  double px=0;
  double py=0;
  bool found = false;
  // Find the idx of the point to be exported close to p
  while(i<Npoints-1)
  {
    double x1 = linestring.getX(i);
    double y1 = linestring.getY(i);
    double x2 = linestring.getX(i+1);
    double y2 = linestring.getY(i+1);
    double deltaL = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    double ratio = (offset-L_processed)/deltaL;
    if(offset>=L_processed && offset<=L_processed+deltaL)
    {
      px = x1+ratio*(x2-x1);
      py = y1+ratio*(y2-y1);
      found = true;
      break;
    }
    ++i;
    L_processed += deltaL;
  };
  if (found) {
    *x = px;
    *y = py;
  } else {
    *x = linestring.getX(Npoints-1);
    *y = linestring.getY(Npoints-1);
  }
};  // locate_point_by_offset

/**
 * added by Diao 18.01.17
 * modified by Can 18.01.19
 * modified by Can 18.03.14
 *
 * @param   offset        input offset(from start node)
 * @param   linestring    input linestring
 * @param   mode          input mode, 0 represent cutoff from start node,
 *                        namely export the part p->end , 1 from endnode
 *                        export the part of start-> p
 * @return  cutoffline    output cutoff linstring, the caller
 *                        should take care of freeing the memory
 */
LineString cutoffseg(double offset, const LineString &linestring, int mode)
{
  LineString cutoffline;
  int Npoints = linestring.getNumPoints();
  if (Npoints==2)
  {
    double x1 = linestring.getX(0);
    double y1 = linestring.getY(0);
    double x2 = linestring.getX(1);
    double y2 = linestring.getY(1);
    double deltaL = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
    double ratio = offset/deltaL;
    double new_x = x1+ratio*(x2-x1);
    double new_y = y1+ratio*(y2-y1);
    if (mode==0) {
      // export p -> end
      //if (1-ratio>0.0001) cutoffline.addPoint(new_x, new_y);
      if (1-ratio>0) cutoffline.addPoint(new_x, new_y);
      cutoffline.addPoint(x2, y2);
    } else {
      // export start -> p
      cutoffline.addPoint(x1, y1);
      //if (1-ratio>0.0001) cutoffline.addPoint(new_x, new_y);
      if (1-ratio>=0) cutoffline.addPoint(new_x, new_y);
    }
  } else {
    double L_processed=0;              // length parsed
    int i = 0;
    int p_idx = 0;
    double px=0;
    double py=0;
    // Find the idx of the point to be exported close to p
    while(i<Npoints-1)
    {
      double x1 = linestring.getX(i);
      double y1 = linestring.getY(i);
      double x2 = linestring.getX(i+1);
      double y2 = linestring.getY(i+1);
      double deltaL = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
      double ratio = (offset-L_processed)/deltaL;
      //if
      if(offset>=L_processed && offset<=L_processed+deltaL)
      {
        px= x1+ratio*(x2-x1);
        py = y1+ratio*(y2-y1);
        break;
      }
      ++i;
      L_processed += deltaL;
    };
    if (offset>L_processed) {
      // The offset value is slightly bigger than the length because
      // of precision
      // implies that px and py are still 0
      px = linestring.getX(i);
      py = linestring.getY(i);
    }
    p_idx = i;
    if (mode==0) {              // export p -> end
      cutoffline.addPoint(px,py);
      for(int j=p_idx+1; j<Npoints; ++j)
      {
        cutoffline.addPoint(linestring.getX(j), linestring.getY(j));
      }
    } else {              // export start -> p
      for(int j=0; j<p_idx+1; ++j)
      {
        cutoffline.addPoint(linestring.getX(j), linestring.getY(j));
      }
      cutoffline.addPoint(px,py);
    }
  }
  return cutoffline;
}; //cutoffseg

} // ALGORITHM
} // MM
#endif /* MM_ALGORITHM_HPP */
