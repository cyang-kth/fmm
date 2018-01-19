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


/**
 * added by Diao 18.01.17
 * @param   offset1        input offset(from start node)
 * @param   offset2        input offset(from start node), should be larger or equal with offset1 
 * @param   linestring     input linestring 
 * @return  cutoffline     output cutoff linstring, the caller should take care of freeing the memory
 */
OGRLineString * cutoffseg_unique(double offset1, double offset2, OGRLineString * linestring)
{
    OGRLineString* cutoffline = new OGRLineString();
    int Npoints = linestring->getNumPoints();
    CS_DEBUG(2) std::cout<< "offset_1: "<<offset1 <<std::endl;
    CS_DEBUG(2) std::cout<< "offset_2: "<<offset2 <<std::endl;
    CS_DEBUG(2) std::cout<< "matching edge piont Num: "<<Npoints <<std::endl;
    if (Npoints==2)
    {
        double x1 = linestring->getX(0);
        double y1 = linestring->getY(0);
        double x2 = linestring->getX(1);
        double y2 = linestring->getY(1);
        double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)); 
        double ratio1 = offset1/L;
        double new_x1 = x1+ratio1*(x2-x1);
        double new_y1 = y1+ratio1*(y2-y1);
        double ratio2 = offset2/L;
        double new_x2 = x1+ratio2*(x2-x1);
        double new_y2 = y1+ratio2*(y2-y1);
        cutoffline->addPoint(new_x1, new_y1);
        cutoffline->addPoint(new_x2, new_y2);
    }
    else
    {
        double length_parsed1=0;
        double length_parsed2=0;
        int restart_id1 = 0;
        int restart_id2 = 0;
        double new_x2 = 0;
        double new_y2 = 0;
        int i = 0;
        int j = 0;
        while(i<Npoints-1)
        {
            double x1 = linestring->getX(i);
            double y1 = linestring->getY(i);
            double x2 = linestring->getX(i+1);
            double y2 = linestring->getY(i+1);
            double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
            length_parsed1 = length_parsed1 + L;
            CS_DEBUG(2) std::cout<< "length_parsed1 : "<<length_parsed1 <<std::endl;
            if(offset1<length_parsed1)
            {
                double ratio1 = (offset1-length_parsed1+L)/L;
                double new_x1 = x1+ratio1*(x2-x1);
                double new_y1 = y1+ratio1*(y2-y1);
                cutoffline->addPoint(new_x1, new_y1);
                restart_id1 = i+1;
                break;
            }
            ++ i;
        };
        while(j<Npoints-1)
        {
            double x1 = linestring->getX(j);
            double y1 = linestring->getY(j);
            double x2 = linestring->getX(j+1);
            double y2 = linestring->getY(j+1);
            double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
            length_parsed2 = length_parsed2 + L;
            CS_DEBUG(2) std::cout<< "length_parsed2 : "<<length_parsed2 <<std::endl;
            // here the "=" is necessay, or there would be instance that new_x2 = new_y2= 0
            // due to the decimal precision probelm, we add 0.01 here (based on the real data test) 
            if(offset2<length_parsed2+0.01)
            {   
                CS_DEBUG(2) std::cout<< "SUCCESSFULLY ENTER"<<std::endl;
                double ratio2 = (offset2-length_parsed2+L)/L;
                new_x2 = x1+ratio2*(x2-x1);
                new_y2 = y1+ratio2*(y2-y1);
                restart_id2 = j+1;
                break;
            }
            ++j;
        };
        CS_DEBUG(2) std::cout<< "restart_1 id: "<<restart_id1 <<std::endl;
        CS_DEBUG(2) std::cout<< "restart_2 id: "<<restart_id2 <<std::endl;
        // restart_id1 =< restart_id2
        if (restart_id1 != restart_id2)
        {   // 
            for(int k=restart_id1; k<restart_id2; ++k)
            {
                cutoffline->addPoint(linestring->getX(k), linestring->getY(k));
            }
        }
        cutoffline->addPoint(new_x2, new_y2);
    }
    return cutoffline;
};//cutoffseg_twoparameters


/**
 * added by Diao 18.01.17
 * modified by Can 18.01.19 
 * @param   offset        input offset(from start node)
 * @param   linestring    input linestring 
 * @param   mode          input mode, 0 represent cutoff from start node, 1 from endnode
 * @return  cutoffline    output cutoff linstring, the caller should take care of freeing the memory
 */
OGRLineString * cutoffseg_unique_v2(double offset1, double offset2, OGRLineString * linestring)
{
    OGRLineString* cutoffline = new OGRLineString();
    int Npoints = linestring->getNumPoints();
    CS_DEBUG(2) std::cout<< "offset_1: "<<offset1 <<std::endl;
    CS_DEBUG(2) std::cout<< "offset_2: "<<offset2 <<std::endl;
    CS_DEBUG(2) std::cout<< "matching edge piont Num: "<<Npoints <<std::endl;
    if (Npoints==2) // A single segment
    {
        double x1 = linestring->getX(0);
        double y1 = linestring->getY(0);
        double x2 = linestring->getX(1);
        double y2 = linestring->getY(1);
        double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)); 
        double ratio1 = offset1/L;
        double new_x1 = x1+ratio1*(x2-x1);
        double new_y1 = y1+ratio1*(y2-y1);
        double ratio2 = offset2/L;
        double new_x2 = x1+ratio2*(x2-x1);
        double new_y2 = y1+ratio2*(y2-y1);
        cutoffline->addPoint(new_x1, new_y1);
        cutoffline->addPoint(new_x2, new_y2);
    }
    else // Multiple segments
    {
        double L = 0; // current length parsed 
        int i = 0;
        while(i<Npoints-1)
        {
            double x1 = linestring->getX(i);
            double y1 = linestring->getY(i);
            double x2 = linestring->getX(i+1);
            double y2 = linestring->getY(i+1);
            double deltaL = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)); // current segment length
            // If L <= offset1 <= L + deltaL
            if (L< offset1 && offset1<L+deltaL) {
                double ratio1 = (offset1-L)/deltaL;
                double new_x1 = x1+ratio1*(x2-x1);
                double new_y1 = y1+ratio1*(y2-y1);
                cutoffline->addPoint(new_x1, new_y1);               
            } 
            // If offset1 < L < offset2
            if (offset1<L && L< offset2){
                cutoffline->addPoint(x1,y1);
            }
            // If L <= offset2 <= L + deltaL
            if (L< offset2 && offset2<L+deltaL) {
                double ratio2 = (offset2-L)/deltaL;
                double new_x2 = x1+ratio2*(x2-x1);
                double new_y2 = y1+ratio2*(y2-y1);
                cutoffline->addPoint(new_x2, new_y2);
            }  
            L = L + deltaL;
            ++i;
        };
    }
    return cutoffline;
};//cutoffseg_twoparameters

/**
 * added by Diao 18.01.17
 * @param   offset        input offset(from start node)
 * @param   linestring    input linestring 
 * @param   mode          input mode, 0 represent cutoff from start node, 1 from endnode
 * @return  cutoffline    output cutoff linstring, the caller should take care of freeing the memory
 */
OGRLineString * cutoffseg(double offset, OGRLineString * linestring, int mode)
{
    OGRLineString* cutoffline = new OGRLineString();
    int Npoints = linestring->getNumPoints();
    if (mode==0)
    {
        if (Npoints==2) 
        {
            double x1 = linestring->getX(0);
            double y1 = linestring->getY(0);
            double x2 = linestring->getX(1);
            double y2 = linestring->getY(1);
            double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));    
            double ratio = offset/L;
            double new_x = x1+ratio*(x2-x1);
            double new_y = y1+ratio*(y2-y1);
            cutoffline->addPoint(new_x, new_y);
            cutoffline->addPoint(x2, y2);
        }
        else
        {
            double length_parsed=0;
            int restart_id = 0;
            int i = 0;
            CS_DEBUG(2)  std::cout<< "offset: "<<offset <<std::endl;
            while(i<Npoints-1)
            {       
                double x1 = linestring->getX(i);
                double y1 = linestring->getY(i);
                double x2 = linestring->getX(i+1);
                double y2 = linestring->getY(i+1);
                double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
                length_parsed = length_parsed + L;
                CS_DEBUG(2) std::cout<< "length of current edge: "<<L <<std::endl;
                CS_DEBUG(2) std::cout<< "length parsed: "<<length_parsed <<std::endl;
                if(offset<length_parsed)
                {
                    double ratio = (offset-length_parsed+L)/L;
                    CS_DEBUG(2) std::cout<< "ratio: "<<ratio <<std::endl;
                    double new_x = x1+ratio*(x2-x1);
                    double new_y = y1+ratio*(y2-y1);
                    cutoffline->addPoint(new_x, new_y);
                    restart_id = i+1;
                    break;
                }
                ++i;
            };
            CS_DEBUG(2) std::cout<< "restart_id: "<<restart_id <<std::endl;
            for(int j=restart_id; j<Npoints; ++j)
            {   
                cutoffline->addPoint(linestring->getX(j), linestring->getY(j));
            }
        }
    }
    else
    {
        if (Npoints==2)
        {
            double x1 = linestring->getX(0);
            double y1 = linestring->getY(0);
            double x2 = linestring->getX(1);
            double y2 = linestring->getY(1);
            double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));   
            double ratio = offset/L;
            double new_x = x1+ratio*(x2-x1);
            double new_y = y1+ratio*(y2-y1);
            CS_DEBUG(2) std::cout<< "final L: "<<L <<std::endl;
            CS_DEBUG(2) std::cout<< "final offset: "<<offset <<std::endl;
            CS_DEBUG(2) std::cout<< "final ratio: "<<ratio <<std::endl;
            cutoffline->addPoint(x1, y1);
            cutoffline->addPoint(new_x, new_y);
        }
        else
        {
            double length_parsed=0;
            int i = 0;
            while(i<Npoints-1)
            {
                double x1 = linestring->getX(i);
                double y1 = linestring->getY(i);
                double x2 = linestring->getX(i+1);
                double y2 = linestring->getY(i+1);
                double L = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));
                cutoffline->addPoint(linestring->getX(i), linestring->getY(i));
                length_parsed = length_parsed + L;
                if(offset<=length_parsed)
                {
                    double ratio = (offset-length_parsed+L)/L;
                    double new_x = x1+ratio*(x2-x1);
                    double new_y = y1+ratio*(y2-y1);
                    cutoffline->addPoint(new_x, new_y);
                    break;
                }
                ++i;
            };
        }
    }
    return cutoffline;
};//cutoffseg

/**
 * added by Diao 18.01.17
 * modified by Can 18.01.19
 * @param   offset        input offset(from start node)
 * @param   linestring    input linestring 
 * @param   mode          input mode, 0 represent cutoff from start node, namely export the part p->end , 1 from endnode
 *                        export the part of start-> p
 * @return  cutoffline    output cutoff linstring, the caller should take care of freeing the memory
 */
OGRLineString * cutoffseg_v2(double offset, OGRLineString * linestring, int mode)
{
    OGRLineString* cutoffline = new OGRLineString();
    int Npoints = linestring->getNumPoints();
    if (Npoints==2) 
    {
        double x1 = linestring->getX(0);
        double y1 = linestring->getY(0);
        double x2 = linestring->getX(1);
        double y2 = linestring->getY(1);
        double deltaL = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1));    
        double ratio = offset/deltaL;
        double new_x = x1+ratio*(x2-x1);
        double new_y = y1+ratio*(y2-y1);
        if (mode==0){
            // export p -> end
            cutoffline->addPoint(new_x, new_y);
            cutoffline->addPoint(x2, y2);    
        } else {
            // export start -> p
            cutoffline->addPoint(x1, y1);
            cutoffline->addPoint(new_x, new_y);
        }
    } else {
        double L=0; // length parsed
        int i = 0; 
        int p_idx = 0;
        double px=0;
        double py=0;
        // Find the idx of the point to be exported close to p 
        while(i<Npoints-1)
        {       
            double x1 = linestring->getX(i);
            double y1 = linestring->getY(i);
            double x2 = linestring->getX(i+1);
            double y2 = linestring->getY(i+1);
            double deltaL = std::sqrt((x2-x1)*(x2-x1)+(y2-y1)*(y2-y1)); // length of current segment
            if(offset>L && offset<L+deltaL)
            {
                double ratio = (offset-L)/deltaL;
                px= x1+ratio*(x2-x1);
                py = y1+ratio*(y2-y1);
                p_idx = i;
                // cutoffline->addPoint(new_x, new_y);
                break;
            }
            ++i;
            L += deltaL; 
        };
        if (mode==0){ // export p -> end
            cutoffline->addPoint(px,py);
            for(int j=p_idx+1; j<Npoints; ++j)
            {   
                cutoffline->addPoint(linestring->getX(j), linestring->getY(j));
            }
        } else { // export start -> p
            for(int j=0; j<p_idx+1; ++j)
            {   
                cutoffline->addPoint(linestring->getX(j), linestring->getY(j));
            }
            cutoffline->addPoint(px,py);
        }   
    }
    return cutoffline;
};//cutoffseg



} // ALGORITHM
} // MM
#endif /* MM_ALGORITHM_HPP */
