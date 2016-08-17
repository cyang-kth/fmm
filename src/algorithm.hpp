/**
 * Content
 * Functions for linear referencing whch finds the closest point on a 
 * a polyline given an input point.
 *      
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_ALGORITHM_HPP
#define MM_ALGORITHM_HPP
#include <cmath>
#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
namespace MM
{
namespace ALGORITHM {

/**
 * Compute the boundary of an OGRLineString and returns the result in 
 * the passed x1,y1,x2,y2 variables.
 * 
 * @param linestring: input, which is a pointer to a 
 * linestring object
 * @param x1,y1,x2,y2: the coordinates of the boundary 
 */
void boundingbox_geometry(OGRLineString *linestring,double *x1,double *y1,double *x2,double *y2)
{
    int Npoints = linestring->getNumPoints();
    *x1 = DBL_MAX;
    *y1 = DBL_MAX;
    *x2 = DBL_MIN;
    *y2 = DBL_MIN;
    double x,y;
    for(int i=0; i<Npoints; ++i)
    {
        x = linestring->getX(i);
        y = linestring->getY(i);
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
void closest_point_on_segment(double x,double y,double x1,double y1,double x2,double y2,double *dist,double *offset)
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
    ratio=(ratio>1)?1:ratio;
    ratio=(ratio<0)?0:ratio;
    double prj_x = x1+ ratio*(x1_x2);
    double prj_y = y1+ ratio*(y1_y2);
    *offset = std::sqrt((prj_x-x1)*(prj_x-x1)+(prj_y-y1)*(prj_y-y1));
    *dist = std::sqrt((prj_x-x)*(prj_x-x)+(prj_y-y)*(prj_y-y));
    // std::cout<<"Ratio is "<<ratio<<" prjx "<<prj_x<<" prjy "<<prj_y<<std::endl;
    CS_DEBUG(2) std::cout<<"Offset is "<<*offset<<" Distance "<<*dist<<std::endl;
}; // closest_point_on_segment


/**
 * A linear referencing function
 * Given a point and a polyline, return the projected distance (p to p')
 * and offset distance (the distance along the polyline from its start
 * to the projected point p') in the passed variables
 * @param point         input point
 * @param linestring    input linestring 
 * @param result_dist   output projected distance 
 * @param result_offset output offset distance from the start of the 
 * polyline
 */
void linear_referencing(OGRPoint *point,OGRLineString *linestring,double *result_dist,float *result_offset)
{
    int Npoints = linestring->getNumPoints();
    double min_dist=DBL_MAX;
    double final_offset=DBL_MAX;
    double length_parsed=0;
    double x = point->getX();
    double y = point->getY();
    int i=0;
    // Iterating to check p(i) == p(i+2)
    int seg_idx=0;
    while(i<Npoints-1)
    {
        double x1 = linestring->getX(i);
        double y1 = linestring->getY(i);
        double x2 = linestring->getX(i+1);
        double y2 = linestring->getY(i+1);
        double temp_min_dist;
        double temp_min_offset;
        CS_DEBUG(3) std::cout<<"\nProcess segment "<<i<<std::endl;
        closest_point_on_segment(x,y,x1,y1,x2,y2,&temp_min_dist,&temp_min_offset);
        if (temp_min_dist<min_dist)
        {
            min_dist=temp_min_dist;
            final_offset = length_parsed+temp_min_offset;
        }
        length_parsed+=std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
        CS_DEBUG(3) std::cout<<"Length parsed "<<length_parsed<<std::endl;
        ++i;
    };
    *result_dist=min_dist;
    *result_offset=final_offset;
}; // linear_referencing

} // ALGORITHM
} // MM
#endif /* MM_ALGORITHM_HPP */
