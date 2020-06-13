### Run FMM on OpenStreetMap

1. Download an OSM file.

```
wget https://download.bbbike.org/osm/bbbike/Stockholm/Stockholm.osm.pbf
```

2. Run map matching

```
stmatch --network Stockholm.osm.pbf --gps traj.csv -k 16 -r 0.005 -e 0.0005 --output mr.txt
```

3. Result

<div align="center">
  <img src="result.png">
</div>
