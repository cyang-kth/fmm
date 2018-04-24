#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>

#include <boost/geometry/index/rtree.hpp>

// to store queries results
#include <vector>

// just for output
#include <iostream>
#include <boost/foreach.hpp>
#include <omp.h>

namespace bg = boost::geometry;
namespace bgi = boost::geometry::index;

int main()
{
    typedef bg::model::point<float, 2, bg::cs::cartesian> point;
    typedef bg::model::box<point> box;
    typedef std::pair<box, unsigned> value;

    // create the rtree using default constructor
    bgi::rtree< value, bgi::quadratic<16> > rtree;

    // create some values
    for ( unsigned i = 0 ; i < 10 ; ++i )
    {
        // create a box
        box b(point(i + 0.0f, i + 0.0f), point(i + 0.5f, i + 0.5f));
        // insert new value
        rtree.insert(std::make_pair(b, i));
    }

    // find values intersecting some area defined by a box

    std::vector<box> boxes;
    boxes.push_back(box(point(0, 0), point(5, 5)));
    boxes.push_back(box(point(1, 1), point(4, 4)));
    boxes.push_back(box(point(2, 2), point(3, 3)));

    #pragma omp parallel for 
    for(int i=0; i < boxes.size(); i++)
    {
        int tid = omp_get_thread_num();
        // Borrowed from here
        // https://stackoverflow.com/questions/4106992/parallelize-output-using-openmp
        std::stringstream buf;
        buf << "---- spatial query from thread: " << tid << "----\n";
        std::vector<value> result_s;
        rtree.query(bgi::intersects(boxes[i]),std::back_inserter(result_s));
        // display results
        buf << "spatial query box:" << "\n";
        buf << bg::wkt<box>(boxes[i]) << "\n";
        buf << "spatial query result:" << "\n";
        BOOST_FOREACH(value const& v, result_s)
            buf << bg::wkt<box>(v.first) << " - " << v.second << "\n";
        // Write the buffer to output
        // #pragma omp critical
        std::cout << buf.rdbuf();
    }
    return 0;
}