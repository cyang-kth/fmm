## About the image

The graph.png file shows the index instead of the ID field of each edge. 

## Input of the algorithm

- Graph.csv: graph topology of the road network
- network.shp: network shapefile used to build R-tree
- gps.csv: GPS trajectories
- podt.csv: paritial OD table for the graph with a specific upper bound
- candidate_list.csv: a candidate list example generated from P1-P6-P2
- edges.shp: the original network file 
- edges_node_gap.shp: same with edges.shp except a node 11 is replaced 
with id of 50. The number of vertices called by the graph object returns
51 in this case. It implies that the number of vertices does not represent
the number of nodes because in the routing process, the precedessor map
and distance map requires to access a vector of nodes.

### ogr-info 
ogr-info can be used to view shapefile information from command.
http://www.gdal.org/ogrinfo.html 

-`-ro`: read-only mode
-`-so`: summary only mode 
-`-al`: List all features of all layers
    
    OGRFeature(edges):29
      _uid_ (String) = 30
      id (String) = 30
      source (String) = 17
      target (String) = 16
      cost (Real) = 1.000000000000000
      x1 (Real) = 3.500000000000000
      y1 (Real) = 2.300000000000000
      x2 (Real) = 3.500000000000000
      y2 (Real) = 4.000000000000000
      LINESTRING (3.5 2.3,3.5 4.0)

Only show meta-data information:
`ogrinfo -ro -so -al edges.shp`

Read the information of specific feature:
`ogrinfo -ro -al edges.shp -fid 17`

The fid information can be read from QGIS in label with record/$id, or in the Derived attribute/feature id, that is guaranteed to be sequentially ordered from 0 to N-1. 

