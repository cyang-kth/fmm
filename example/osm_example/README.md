### Run FMM on OpenStreetMap

1. Download an OSM file.

Download from the [Geofabrik](https://download.geofabrik.de/index.html), clip for a smaller region using [osmconvert](https://wiki.openstreetmap.org/wiki/Osmconvert):  

```
osmconvert country.osm.pbf -b=west_coor,south_coor,east_coor,north_coor -o=region.o5m
```

Note the coordinate of the box is in order of WSEN.

Download from other online resources of OSM.

```
# On Ubuntu
wget https://download.bbbike.org/osm/bbbike/Stockholm/Stockholm.osm.pbf
# On Windows and Mac
curl https://download.bbbike.org/osm/bbbike/Stockholm/Stockholm.osm.pbf -o Stockholm.osm.pbf
```

2. Run map matching

```
stmatch --network Stockholm.osm.pbf --gps traj.csv -k 16 -r 0.005 -e 0.0005 --vmax 0.0002 --output mr.txt
```

3. Result

<div align="center">
  <img src="result.png" width="400">
</div>
