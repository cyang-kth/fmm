### FMM OpenStreetMap example

In order to run this demo, you need to prepare an OSM file and put it in this folder.

#### Prepare an OSM dataset

Extract only highway road network

```
osmconvert Stockholm.osm.pbf -o=stockholm.o5m
```

#### Run jupyter-notebook

Type

```
jupyter-notebook
```

Visit the notebook link in your browser.

#### Run OSM for command line application

stmatch --network stockholm.o5m --gps ../data/trips.shp -k 4 -r 0.003 -e 0.0005 --output mr.txt
