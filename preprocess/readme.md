### Download road network from OSM using osmnx

Install osmnx 
    
    pip install osmnx

Download the road network of a city to shapefile using osmnx:

    import osmnx as ox
    place ="Shenzhen, China"
    G = ox.graph_from_place(place, network_type='drive')
    ox.save_graph_shapefile(G, filename='shenzhen_network')

The process may take several mintues. 

It will generate two shapefiles `edges` and `nodes`. The `edges` shapefile already contains the topology of the road network (the attribute from,to node). 

The configuration for generating ubodt can be 

    <?xml version="1.0" encoding="utf-8"?>
    <ubodt_config>
        <input>
            <network>
                <file>edges.shp</file>
                <id>osmid</id>
                <source>from</source>
                <target>to</target>
            </network>
        </input>
        <parameters>
            <delta>0.01</delta>
        </parameters>
        <output>
            <file>ubodt.txt</file>        
        </output>
    </ubodt_config>


### Complement a road network with reversed link for bidirectional road

OSM road network generally contains an attribute called one-way to indicate the direction of an edge. To consider this information, a reverse edge should be added to the road network wherever one-way is false. 

This can be done easily in PostGIS database using the ST_Reverse function.

The following sql codes are provided as a reference.

```

--- Import shapefile into a table in PostGIS database (run in bash shell). 

shp2pgsql network.shp network.original | psql -U USER_NAME -d DATABASE_NAME

--- This command import network.shp into PostGIS database as a table `network.original`, which means schema.tablename. 

--- Create a new table to store the dual version (run in psql)
CREATE TABLE network.dual
(
  gid serial NOT NULL,
  geom geometry(LineString),
  source integer,
  target integer,
  cost double precision,
  original_id integer,
  CONSTRAINT dual_pkey PRIMARY KEY (gid)
);

-- Insert original edge (run in psql)
INSERT INTO 
    network.dual (geom,source,target,cost,original_id)
SELECT ST_LineMerge(geom),source,target,distance,gid FROM network.original;

-- Insert reversed edge (run in psql)
INSERT INTO 
    network.dual (geom,source,target,cost,original_id)
SELECT ST_Reverse(ST_LineMerge(geom)),target, source, distance, gid FROM network.original where one_way=="false";

-- Export to a shapefile network_dual.shp (run in bash shell)

pgsql2shp -f network_dual.shp -h localhost -u USER_NAME -d DATABASE_NAME "SELECT gid::integer as id,source,target,geom from network.dual"

```

### Creating topology for a shapefile

If you already have a road network file in GDAL supported formats, e.g., ESRI shp, GeoJSON or CSV without topology information (id, source and target fields). Spatial database [PostGIS](http://postgis.net/) and its extension [pgRouting](http://pgrouting.org/) can solve the problem:

1. [Add shapefiles to PostGIS database](https://gis.stackexchange.com/questions/41799/adding-shapefiles-to-postgis-database)
2. [Create topology of road network with the function pgr_createTopology in pgrouting](http://docs.pgrouting.org/2.2/en/src/topology/doc/pgr_createTopology.html)
3. [Export PostGIS table to shapefile](https://gis.stackexchange.com/questions/55206/how-can-i-get-a-shapefile-from-a-postgis-query)

### References

- https://github.com/gboeing/osmnx-examples/blob/master/notebooks/05-example-save-load-networks-shapes.ipynb
- https://github.com/gboeing/osmnx
- https://github.com/gboeing/osmnx-examples



