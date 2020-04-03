### Web demo of FMM

To run this demo, you need to first [install the python api](https://fmm-wiki.github.io/docs/installation/) of fmm.

- fmm
- flask
- tornado

Install with pip

```bash
pip install flask tornado
```

### Run the demo

Try to preprare real world routable road network file (e.g., network.shp).
Then update the json configuration below.

Run the fmm web demo
```bash
python web_demo.py -c fmm_config.json
```

Run the stmatch web demo

```bash
python web_demo.py -c stmatch_config.json
```

Visit `http://localhost:5000/demo` in your browser.

A practical tutorial is provided at [osm map matching](https://github.com/cyang-kth/osm_mapmatching).
