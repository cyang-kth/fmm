/**
 * Content
 * Definition of the Network class
 *
 * @author: Can Yang
 * @version: 2017.11.11
 */

#ifndef MM_NETWORK_HPP
#define MM_NETWORK_HPP
#include "gdal/ogrsf_frmts.h" // C++ API for GDAL
#include <iostream>
#include <math.h> // Calulating probability
#include "multilevel_debug.h"
#include "types.hpp"
#include "util.hpp"
#include "algorithm.hpp"
#include "gps.hpp"
#include <iomanip>
#include <algorithm> // Partial sort copy
#include <unordered_set> // Partial sort copy
// Data structures for Rtree
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>
#include <boost/function_output_iterator.hpp>

namespace MM
{

// Define a type alias for the rtree used in map matching
/* Edge format in the network */
typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> boost_point; // Point for rtree box
// typedef boost::geometry::model::linestring<boost_point> boost_linestring; // Point for rtree box
typedef boost::geometry::model::box<boost_point> boost_box; // Point for rtree box
typedef std::pair<boost_box,Edge*> Item; // Item stored in rtree
typedef boost::geometry::index::rtree<Item,boost::geometry::index::quadratic<16>> Rtree; // Rtree definition

// This function is used for KNN sort
static bool candidate_compare(const Candidate &a, const Candidate &b)
{
    if (a.dist!=b.dist)
    {
        return a.dist<b.dist;
    }
    else
    {
        return a.edge->id<b.edge->id;
    }
};

class Network
{
public:
    /**
     *  Constructor of Network
     *
     *  A vector of network edges will be constructed
     *
     *  @param filename: the path to a network file in ESRI shapefile format
     *  @param id_name: the name of the id attribute in the network file, "id" by default.
     *  @param source_name: the name of the source attribute in the network file, "source" by default.
     *  @param target_name: the name of the target attribute in the network file, "target" by default.
     *
     */
    Network(const std::string &filename,
            const std::string &id_name="id",
            const std::string &source_name="source",
            const std::string &target_name="target")
    {
        std::cout<<"Read network shp file from " << filename <<'\n';
        std::cout<<"Id column "<<id_name <<'\n';
        OGRRegisterAll();
#if GDAL_VERSION_MAJOR < 2
        OGRDataSource *poDS = OGRSFDriverRegistrar::Open(filename.c_str());
#else
        GDALDataset *poDS = (GDALDataset*) GDALOpenEx(filename.c_str(), GDAL_OF_VECTOR, NULL, NULL, NULL );
#endif // GDAL_VERSION_MAJOR
        if( poDS == NULL )
        {
            printf( "Open failed.\n" );
            exit( 1 );
        }
        OGRLayer  *ogrlayer = poDS->GetLayer(0);
        int NUM_FEATURES = ogrlayer->GetFeatureCount();
        std::cout<< "\tNumber of edges in file: " << NUM_FEATURES << '\n';
        network_edges= std::vector<Edge>(NUM_FEATURES);
        // Initialize network edges
        OGRFeatureDefn *ogrFDefn = ogrlayer->GetLayerDefn();
        OGRFeature *ogrFeature;
        // Fetch the field index given field name.
        int id_idx=ogrFDefn->GetFieldIndex(id_name.c_str());
        int source_idx=ogrFDefn->GetFieldIndex(source_name.c_str());
        int target_idx=ogrFDefn->GetFieldIndex(target_name.c_str());
        std::cout<< "\tSHP ID idx: " <<id_idx<< '\n';
        std::cout<< "\tSHP source idx: " <<source_idx<< '\n';
        std::cout<< "\tSHP target idx: " <<target_idx<< '\n';
        if (source_idx<0||target_idx<0||id_idx<0)
        {
            std::cout<<std::setw(12)<<""<< "id, source or target column not found "<< '\n';
#if GDAL_VERSION_MAJOR < 2
            OGRDataSource::DestroyDataSource( poDS );
#else
            GDALClose( poDS );
#endif // GDAL_VERSION_MAJOR
            std::exit(EXIT_FAILURE);
        }

        if (wkbFlatten(ogrFDefn->GetGeomType()) != wkbLineString)
        {
			std::cout<<std::setw(12)<<""<< "Geometry type of network is " <<OGRGeometryTypeToName(ogrFDefn->GetGeomType())<<'\n';
            std::cout<<std::setw(12)<<""<< "It should be LineString"<< '\n';
#if GDAL_VERSION_MAJOR < 2
            OGRDataSource::DestroyDataSource( poDS );
#else
            GDALClose( poDS );
#endif // GDAL_VERSION_MAJOR
            std::cout<<"Program stop"<< '\n';
            std::exit(EXIT_FAILURE);
        } else {
			std::cout<< "\tGeometry type is " <<OGRGeometryTypeToName(ogrFDefn->GetGeomType())<<'\n';
        }
        OGRSpatialReference *ogrsr = ogrFDefn->GetGeomFieldDefn(0)->GetSpatialRef();
        if (ogrsr != nullptr) {
			srid = ogrsr->GetEPSGGeogCS();
			if (srid==-1)
			{
				srid= 4326;
				std::cout<< "\t---- Warning: srid is not found, set to 4326 for default"<< '\n';
			} else {
				std::cout<< "\tSRID is "<<srid<< '\n';
			}
        } else {
			srid= 4326;
            std::cout<< "\t---- Warning: srid is not found, set to 4326 for default"<< '\n';
        }
        std::unordered_set<int> nodeSet;
        while( (ogrFeature = ogrlayer->GetNextFeature()) != NULL)
        {
            int id = ogrFeature->GetFID();
            Edge *e = &network_edges[id];
            e->id = id;
            e->id_attr = std::string(ogrFeature->GetFieldAsString(id_idx));
            e->source = ogrFeature->GetFieldAsInteger(source_idx);
            e->target = ogrFeature->GetFieldAsInteger(target_idx);
            OGRGeometry *rawgeometry = ogrFeature->GetGeometryRef();
//             CS_DEBUG(3) std::cout<<"Line "<< __LINE__<<" ID "<< e->id <<" Length "<<((OGRLineString*) rawgeometry)->get_Length()<<"\n";
//             CS_DEBUG(3) std::cout<<"Line "<< __LINE__<<" ID "<< e->id <<" Points "<<((OGRLineString*) rawgeometry)->getNumPoints()<<"\n";
            // The cloned geometry has to be freed by OGRGeometryFactory
            // https://github.com/OSGeo/gdal/blob/93fb17379bccba28a43a03bb2c19b868f264ebe1/gdal/ogr/ogrlinestring.cpp#L141
#ifdef USE_BG_GEOMETRY
            e->geom = ogr2bg((OGRLineString*) rawgeometry);
#else
            e->geom = (OGRLineString*) rawgeometry->clone();
#endif
            e->length = e->geom->get_Length();
            if (e->source>max_node_id)
            {
                max_node_id = e->source;
            }
            if (e->target>max_node_id)
            {
                max_node_id = e->target;
            }
            if (nodeSet.find(e->source)==nodeSet.end()){
                nodeSet.insert(e->source);
            }
            if (nodeSet.find(e->target)==nodeSet.end()){
                nodeSet.insert(e->target);
            }
            OGRFeature::DestroyFeature(ogrFeature);
        }
#if GDAL_VERSION_MAJOR < 2
        OGRDataSource::DestroyDataSource( poDS );
#else
        GDALClose( poDS );
#endif // GDAL_VERSION_MAJOR
        CS_DEBUG(3) std::cout<<"Line "<< __LINE__<< " Length "<<network_edges[0].geom->get_Length()<<"\n";
        std::cout<<"Read network finish."<< '\n';
        node_count = nodeSet.size();
        std::cout<<"\tThe maximum node ID is "<< max_node_id << '\n';
        std::cout<<"Node count is "<< node_count << '\n';
        std::cout<<"\tTotal number of edges read "<< network_edges.size()<< '\n';
    }; // Network constructor
    ~Network()
    {
        std::cout<< "Cleaning network" << '\n';
        for (auto &item:network_edges)
        {
#ifdef USE_BG_GEOMETRY
            delete item.geom;
#else
            OGRGeometryFactory::destroyGeometry(item.geom);
#endif
        }
        std::cout<< "Cleaning network finished" << '\n';
    }

    int get_node_count(){
        return node_count;
    };
    // Get the edge vector
    std::vector<Edge> *get_edges()
    {
        return &network_edges;
    };
    // Get the ID attribute of an edge according to its index
    std::string get_edge_id_attr(int eid)
    {
        return network_edges[eid].id_attr;
    };
    int get_max_node_id()
    {
        return max_node_id;
    };
    // Construct a Rtree using the vector of edges
    void build_rtree_index()
    {
        // Build an rtree for candidate search
        std::cout<<"Start to construct boost rtree"<<'\n';
        // create some Items
        for (std::size_t i = 0 ; i < network_edges.size(); ++i)
        {
            // create a boost_box
            Edge *edge = &network_edges[i];
            CS_DEBUG(3) std::cout<<"Number of points is "<<edge->geom->getNumPoints()<<"\n";
            // boundary is returned is a multipoint geometry, but not the envelop
            // instead, it is only the first and last point
            double x1,y1,x2,y2;
            ALGORITHM::boundingbox_geometry(edge->geom,&x1,&y1,&x2,&y2);
            CS_DEBUG(3) std::cout<<"Process trajectory: "<<i<<'\n';
            CS_DEBUG(3) std::cout<<"x1,y1,x2,y2: "<<x1<<","<<y1<<","<<x2<<","<<y2<<'\n';
            boost_box b(boost_point(x1,y1), boost_point(x2,y2));
            rtree.insert(std::make_pair(b,edge));
        }
        std::cout<<"Finish construct boost rtree"<<'\n';
    };
    /**
     *  Search for k nearest neighboring (KNN) candidates of a
     *  trajectory within a search radius
     *
     *  @param trajectory: input trajectory
     *  @param k: the number of coordindates
     *  @param r: the search radius
     *  @return Traj_Candidates: a 2D vector of Candidates containing
     *  the candidates selected for each point in a trajectory
     *
     */
    Traj_Candidates search_tr_cs_knn(Trajectory &trajectory,std::size_t k,double radius){
        return search_tr_cs_knn(trajectory.geom,k,radius);
    }

    /**
     *  Search for k nearest neighboring (KNN) candidates of a
     *  linestring within a search radius
     */
    Traj_Candidates search_tr_cs_knn(LineString *geom,std::size_t k,double radius)
    {
        int NumberPoints = geom->getNumPoints();
        Traj_Candidates tr_cs(NumberPoints);
        for (int i=0; i<NumberPoints; ++i)
        {
            CS_DEBUG(2) std::cout<<"Search candidates for point index "<<i<< '\n';
            // Construct a bounding boost_box
            double px = geom->getX(i);
            double py = geom->getY(i);
            Point_Candidates pcs;
            boost_box b(boost_point(geom->getX(i)-radius,geom->getY(i)-radius),boost_point(geom->getX(i)+radius,geom->getY(i)+radius));
            std::vector<Item> temp;
            // Rtree can only detect intersect with a the bounding box of the geometry stored.
            rtree.query(boost::geometry::index::intersects(b),std::back_inserter(temp));
            for (Item &i:temp)
            {
                // Check for detailed intersection
                // The two edges are all in OGR_linestring
                Edge *edge = i.second;
                float offset;
                double dist;
                ALGORITHM::linear_referencing(px,py,edge->geom,&dist,&offset);
                CS_DEBUG(2) std::cout<<"Edge id: "<<edge->id<< '\n';
                CS_DEBUG(2) std::cout<<"Dist: "<<dist<< '\n';
                CS_DEBUG(2) std::cout<<"Offset: "<<offset<< '\n';
                if (dist<=radius)
                {
                    Candidate c = {offset,dist,Network::emission_prob(dist),edge,NULL,0,0};
                    pcs.push_back(c);
                }
            }
            if (pcs.empty())
            {
                return Traj_Candidates();
            };
            // KNN part
            if (pcs.size()<=k)
            {
                tr_cs[i]=pcs;
            }
            else
            {
                // Find the KNN neighbors
                CS_DEBUG(2) std::cout<<"Perform KNN search"<< '\n';
                tr_cs[i]=Point_Candidates(k);
                std::partial_sort_copy(
                    pcs.begin(),pcs.end(),
                    tr_cs[i].begin(),tr_cs[i].end(),
                    candidate_compare);
            }
        }
        CS_DEBUG(1) UTIL::print_traj_candidates_summary(tr_cs);
        CS_DEBUG(2) UTIL::print_traj_candidates(tr_cs);
        return tr_cs;
    };

    /**
     * Added in 2018.01.17, by Diao
     * Modified in 2018.01.19 by Can
     *
     * a modified version of geometry path construction,
     * which the offset is taken into consideration, and the
     * final path with a correct start point and end point corresponding
     * to the original trajectory. Namely at both the first and last edge
     * in the complete path, only the part traversed is exported.
     *
     * Construct the geometry of a complete path,
     * @param o_path_ptr: a pointer to the o_path_ptr, where each element
     * is a Candidate object. This input is needed because the offset is used to
     * split the first and last edge in the complete path.
     * @param  complete_path: a pointer to a complete path, where each element
     * is an integer representing spatial contiguous edge index
     * @return  a pointer to the geometry of the complete path, The
     * caller should take care of freeing its memory.
     */
    LineString *complete_path_to_geometry(O_Path *o_path_ptr, C_Path *complete_path)
    {
        if (complete_path==nullptr || complete_path->empty()) return nullptr;
        // if (complete_path->empty()) return nullptr;
        LineString *line = new LineString();
        int NOsegs = o_path_ptr->size();
        int NCsegs = complete_path->size();
        GC_DEBUG(2) std::cout<< __FILE__ << __LINE__ <<" Optimal path size "<<NOsegs <<'\n';
        GC_DEBUG(2) std::cout<< __FILE__ << __LINE__ <<" Complete path size "<<NCsegs <<'\n';
        if (NCsegs ==1)
        {
            double firstoffset = (*o_path_ptr)[0]->offset;
            double lastoffset = (*o_path_ptr)[NOsegs-1]->offset;
            GC_DEBUG(2) std::cout<< "first offset " << firstoffset <<'\n';
            GC_DEBUG(2) std::cout<< "last offset " << lastoffset <<'\n';
            LineString * firstseg = network_edges[(*complete_path)[0]].geom;
            LineString * firstlineseg= ALGORITHM::cutoffseg_unique(firstoffset,lastoffset,firstseg);
            append_segs_to_line(line,firstlineseg,0);
            GC_DEBUG(2) UTIL::print_geometry(firstlineseg);
            // Free the memory
            delete firstlineseg;
        } else {
            double firstoffset = (*o_path_ptr)[0]->offset;
            double lastoffset = (*o_path_ptr)[NOsegs-1]->offset;
            LineString * firstseg = network_edges[(*complete_path)[0]].geom;
            LineString * lastseg = network_edges[(*complete_path)[NCsegs-1]].geom;
            LineString * firstlineseg= ALGORITHM::cutoffseg(firstoffset, firstseg, 0);
            LineString * lastlineseg= ALGORITHM::cutoffseg(lastoffset, lastseg, 1);
            GC_DEBUG(2) std::cout<< "First offset " << firstoffset <<'\n';
            GC_DEBUG(2) std::cout<< "First line " <<'\n';
            GC_DEBUG(2) UTIL::print_geometry(firstseg);
            GC_DEBUG(2) std::cout<< "First line cutoff " <<'\n';
            GC_DEBUG(2) UTIL::print_geometry(firstlineseg);
            GC_DEBUG(2) std::cout<< "last offset " << lastoffset <<'\n';
            GC_DEBUG(2) std::cout<< "Last line " <<'\n';
            GC_DEBUG(2) UTIL::print_geometry(lastseg);
            GC_DEBUG(2) std::cout<< "Last line cutoff " <<'\n';
            GC_DEBUG(2) UTIL::print_geometry(lastlineseg);
            append_segs_to_line(line,firstlineseg,0);
            if (NCsegs>2)
            {
                for(int i=1;i<NCsegs-1;++i)
                {
                    LineString * middleseg = network_edges[(*complete_path)[i]].geom;
                    append_segs_to_line(line,middleseg,1);
                }
            };
            append_segs_to_line(line,lastlineseg,1);
            // Free the memory
            delete firstlineseg;
            delete lastlineseg;
        }
        GC_DEBUG(2) std::cout<< "Export result" <<'\n';
        GC_DEBUG(2) UTIL::print_geometry(line);
        return line;
    };


    /**
     * Detect if a complete path contains reverse movement
     *
     * @param  c_path_ptr : a pointer to a complete path
     */
    bool is_complete_path_reverse(C_Path *c_path_ptr)
    {
        if (c_path_ptr==nullptr) return false;
        C_Path &c_path=*c_path_ptr;
        int N = c_path.size();
        bool result=false;
        for(int i=0; i<N-1; ++i)
        {
            int ea = c_path[i];
            int eb = c_path[i+1];
            if (network_edges[ea].source==network_edges[eb].target)
            {
                result=true;
                break;
            }
        }
        return result;
    };
    /**
     * Calculate the emission probability
     * @param  dist:the distance a GPS point to a candidate point
     * @return  the emission probability
     */
    static double emission_prob(double dist)
    {
        double a = dist / GPS_ERROR;
        return exp(-0.5 * a * a); // The unit should be in range of 0,1
    };

    static double emission_prob_to_dist(double eprob){
        return sqrt(-2 * log(eprob))*GPS_ERROR;
    };
private:
    /**
     * Concatenate a linestring segs to a linestring line, used in the
     * function complete_path_to_geometry
     *
     * @param line: pointer to a linestring which will be updated
     * @param segs: pointer to a linestring
     * @param offset: the number of points skipped in segs.
     */
    static void append_segs_to_line(LineString *line,LineString *segs,int offset=0)
    {
        int Npoints = segs->getNumPoints();
        DEBUG(2) std::cout<< "Number of points "<< Npoints <<'\n';
        for(int i=0; i<Npoints; ++i)
        {
            if (i>=offset)
            {
                line->addPoint(segs->getX(i),segs->getY(i));
            }
        }
    };
    int srid; // Spatial reference id
    Rtree rtree; // Network rtree structure
    std::vector<Edge> network_edges; // all edges in the network
    int max_node_id = 0; // a variable to record the maximum node ID
    int node_count = 0;
}; // Network
} // MM
#endif /* MM_NETWORK_HPP */
