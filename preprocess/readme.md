### Download road network from OSM using osmnx

Install osmnx 
    
    pip install osmnx

Download the road network of a city to shapefile using osmnx:

    import osmnx as ox
    place ="Shenzhen, China"
    G = ox.graph_from_place(place, network_type='drive')
    ox.save_graph_shapefile(G, filename='shenzhen_network')

The process may take several mintues. 

### Creating topology for a shapefile

If you already have a road network file in GDAL supported formats, e.g., ESRI shp, GeoJSON and CSV, you may encounter a problem of creating topology of the network, namely, defining id, source and target fields. Spatial database [PostGIS](http://postgis.net/) and its extension [pgRouting](http://pgrouting.org/) can solve the problem:

1. [Add shapefiles to PostGIS database](https://gis.stackexchange.com/questions/41799/adding-shapefiles-to-postgis-database)
2. [Create topology of road network with the function pgr_createTopology in pgrouting](http://docs.pgrouting.org/2.2/en/src/topology/doc/pgr_createTopology.html)
3. [Export PostGIS table to shapefile](https://gis.stackexchange.com/questions/55206/how-can-i-get-a-shapefile-from-a-postgis-query)

### References

- https://github.com/gboeing/osmnx-examples/blob/master/notebooks/05-example-save-load-networks-shapes.ipynb
- https://github.com/gboeing/osmnx
- https://github.com/gboeing/osmnx-examples



