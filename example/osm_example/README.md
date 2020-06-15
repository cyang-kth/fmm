### Run FMM on OpenStreetMap

1. Download an OSM file.

```
# On Ubuntu
wget https://download.bbbike.org/osm/bbbike/Stockholm/Stockholm.osm.pbf
# On Windows and Mac
curl https://download.bbbike.org/osm/bbbike/Stockholm/Stockholm.osm.pbf -o Stockholm.osm.pbf
```

Download data for a small region using [Overpass API](https://overpass-turbo.eu/)

```
[out:xml]
[timeout:25]
;
(
  node
    ["highway"]
    (33.913520126901,-118.22945594788,33.960874396538,-118.1652545929);
  way
    ["highway"]
    (33.913520126901,-118.22945594788,33.960874396538,-118.1652545929);
  relation
    ["highway"]
    (33.913520126901,-118.22945594788,33.960874396538,-118.1652545929);
);
out;
>;
out skel qt;
```

```
osmconvert socal-latest.osm.pbf -b=-118.22945594788,33.913520126901,-118.1652545929,33.960874396538 -o=network.osm
```


2. Run map matching

```
stmatch --network Stockholm.osm.pbf --gps traj.csv -k 16 -r 0.005 -e 0.0005 --vmax 0.0002 --output mr.txt
```

3. Result

<div align="center">
  <img src="result.png" width="400">
</div>
