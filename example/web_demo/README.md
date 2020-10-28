### Web demo of FMM

To run this demo, you need to first [install the python api](https://fmm-wiki.github.io/docs/installation/) of fmm.

- fmm
- flask
- tornado

Install with pip

```bash
pip install flask tornado
```

### Download routable shapefile

Download a routable network shapefile (with id, source, target fields)
from OSM using osmnx following the tutorial [osm map matching](https://github.com/cyang-kth/osm_mapmatching).

An example dataset is provided as [data.tar.gz](data/data.tar.gz).

```
# Extract the dataset
tar -xvf data/data.tar.gz -C data
```

### Run the fmm web demo

```bash
# Precompute UBODT file
ubodt_gen --network data/edges.shp --network_id fid --source u --target v \
--delta 0.03 -o data/ubodt.txt --use_omp
# Start the web app
python web_demo.py -c fmm_config.json
```

### Run the stmatch web demo

```bash
# Start the web app
python web_demo.py -c stmatch_config.json
```

Visit `http://localhost:5000/demo` in your browser.

You should be able to see

![demo](https://github.com/cyang-kth/fmm/blob/master/img/demo3.gif?raw=true)
