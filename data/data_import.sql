-- Graph information
SELECT id,source,target,cost FROM test.edge_table_di


-- Copy to file from SQL
-- chmod a+w /home/gowithwind/workspace/c/geos/data
COPY 
	(SELECT id,source,target,cost FROM test.edge_table_di) 
TO 
	'/home/gowithwind/workspace/c/geos/data/small_graph.csv'
(format csv, HEADER,delimiter ';');

COPY 
(
	SELECT 
		a.path[1] as step,row_number() over (PARTITION BY path) ::int AS id,b.*
	FROM ST_DumpPoints(
		ST_Makeline(ARRAY [ST_Makepoint(1.8,0.4),ST_Makepoint(2.2,1.7),ST_Makepoint(4.2,2.4)])
	) a,test.tg_candidate(a.geom,0.5,1) b
)
TO 
	'/home/gowithwind/workspace/c/geos/data/candidate_list.csv'
(format csv, HEADER,delimiter ';');

/*
ogr2ogr -f "ESRI Shapefile" -a_srs "EPSG:4326" data/poi.shp \
PG:"host=localhost user=postgres dbname=stockholm_gps password=yc1990" \
-sql "SELECT pid,x,y,edge_id,side,fraction,the_geom FROM test.pointsofinterest"
*/
-- ogrinfo -al data/poi.shp

-- Query returned successfully: 4285886 rows affected, 10393 ms execution time.
COPY 
    (SELECT * FROM mapmatch.od_5km) 
TO 
    '/home/gowithwind/workspace/c/geos/input/podt.csv'
(format csv, HEADER,delimiter ';');