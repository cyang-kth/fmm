## Example to test implementation for reading OSM data directly

These instructions were tested on Ubuntu 18.04, after building the code in the OSM branch. They assume a working directory under ``example/command_line_example/``

### 1. Download test data

```
mkdir osm_test
cd osm_test
wget https://www.dropbox.com/s/a3d8hmtkflckw10/fmm_osm_test_data.zip
unzip fmm_osm_test_data.zip
```

Test data contains:
 - OSM data for Singapore (defined by bounding box)
 - Road network in shapefile format (converted from OSM with https://github.com/dkondor/osmconvert/ )
 - Test data with 5 trajectories (created manually by clicking on the map in QGIS)


### 2. Run stmatch with shapefile input

```
../../../build/stmatch --network ways.shp --gps test_points.csv --output stmatch_result.csv -k 4 -r 0.4 -e 0.5 --gps_geom WKT --gps_id id --source from --target to
```

### 3. Run stmatch with OSM input

```
../../../build/stmatch --network Singapore.osm.pbf --gps test_points.csv --output stmatch_result_osm.csv -k 4 -r 0.4 -e 0.5 --gps_geom WKT --gps_id id
```

### 4. Compare results

Unfortunately, it's not possible to compare the list of edge IDs in the output, since these are assigned separately by the two methods. It is possible to compare the output trajectory directly instead:

```
cut -d \; -f 1,3 stmatch_result.csv > stmatch_result2.csv
cut -d \; -f 1,3 stmatch_result_osm.csv > stmatch_result_osm2.csv
diff -q stmatch_result2.csv stmatch_result_osm2.csv
```

No output from the last command means that the output trajectories are identical.
