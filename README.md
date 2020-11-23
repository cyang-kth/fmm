<div align="center">
  <img src="img/fmm_social.jpg">
</div>

| Linux / macOS | Windows | Wiki          | Docs        |
| ------------- | ------- | ------------- | ----------- |
| [![Build Status](https://travis-ci.org/cyang-kth/fmm.svg?branch=master)](https://travis-ci.org/github/cyang-kth/fmm) | [![Build status](https://ci.appveyor.com/api/projects/status/8qee5c8iay75j1am?svg=true)](https://ci.appveyor.com/project/cyang-kth/fmm) | [![Wiki](https://img.shields.io/badge/wiki-website-blue.svg)](https://fmm-wiki.github.io/) | [![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://cyang-kth.github.io/fmm/) |

FMM is an open source map matching framework in C++ and Python. It solves the problem of matching noisy GPS data to a road network. The design considers maximizing performance, scalability and functionality.

### Online demo

Check the [online demo](https://fmm-demo.herokuapp.com/).

### Features

- **High performance**: C++ implementation using Rtree, optimized routing, parallel computing (OpenMP).
- **Python API**: [jupyter-notebook](example/notebook) and [web app](example/web_demo)
- **Scalibility**: millions of GPS points and millions of road edges.  
- **Multiple data format**:
  - Road network in OpenStreetMap or ESRI shapefile.
  - GPS data in Point CSV, Trajectory CSV and Trajectory Shapefile ([more details](https://fmm-wiki.github.io/docs/documentation/input/#gps-data)).
- **Detailed matching information**: traversed path, geometry, individual matched edges, GPS error, etc. More information at [here](https://fmm-wiki.github.io/docs/documentation/output/).
- **Multiple algorithms**: [FMM](http://www.tandfonline.com/doi/full/10.1080/13658816.2017.1400548) (for small and middle scale network) and [STMatch](https://dl.acm.org/doi/abs/10.1145/1653771.1653820) (for large scale road network)
- **Platform support**: Unix (ubuntu) , Mac and Windows(cygwin environment).
- **Hexagon match**: :tada: Match to the uber's [h3](https://github.com/uber/h3) Hexagonal Hierarchical Geospatial Indexing System. Check the [demo](example/h3).

We encourage contribution with feature request, bug report or developping new map matching algorithms using the framework.

### Screenshots of notebook

Map match to OSM road network by drawing

![fmm_draw](https://github.com/cyang-kth/fmm-examples/blob/master/img/fmm_draw.gif?raw=true)

Explore the factor of candidate size k, search radius and GPS error

![fmm_explore](https://github.com/cyang-kth/fmm-examples/blob/master/img/fmm_explore.gif?raw=true)

Explore detailed map matching information

![fmm_detail](https://github.com/cyang-kth/fmm-examples/blob/master/img/fmm_detail.gif?raw=true)

Explore with dual map

![dual_map](https://github.com/cyang-kth/fmm-examples/blob/master/img/dual_map.gif?raw=true)

Map match to hexagon by drawing

![hex_draw](https://github.com/cyang-kth/fmm-examples/blob/master/img/hex_draw.gif?raw=true)

Explore the factor of hexagon level and interpolate

![hex_explore](https://github.com/cyang-kth/fmm-examples/blob/master/img/hex_explore.gif?raw=true)

Source code of these screenshots are available at https://github.com/cyang-kth/fmm-examples.

### Installation, example, tutorial and API.

- Check [https://fmm-wiki.github.io/](https://fmm-wiki.github.io/) for installation, documentation.
- Check [example](example) for simple examples of fmm.
- :tada: Check [https://github.com/cyang-kth/fmm-examples](https://github.com/cyang-kth/fmm-examples)
for interactive map matching in notebook.

### Code docs for developer

Check [https://cyang-kth.github.io/fmm/](https://cyang-kth.github.io/fmm/)

### Contact and citation

Can Yang, Ph.D. student at KTH, Royal Institute of Technology in Sweden

Email: cyang(at)kth.se

Homepage: https://people.kth.se/~cyang/

FMM originates from an implementation of this paper [Fast map matching, an algorithm integrating hidden Markov model with precomputation](http://www.tandfonline.com/doi/full/10.1080/13658816.2017.1400548). A post-print version of the paper can be downloaded at [link](https://people.kth.se/~cyang/bib/fmm.pdf). Substaintial new features have been added compared with the original paper.  

Please cite fmm in your publications if it helps your research:

    Can Yang & Gyozo Gidofalvi (2018) Fast map matching, an algorithm
    integrating hidden Markov model with precomputation, International Journal of Geographical Information Science, 32:3, 547-570, DOI: 10.1080/13658816.2017.1400548

Bibtex file

    @article{doi:10.1080/13658816.2017.1400548,
    author = {Can Yang and Gyozo Gidofalvi},
    title = {Fast map matching, an algorithm integrating hidden Markov model with precomputation},
    journal = {International Journal of Geographical Information Science},
    volume = {32},
    number = {3},
    pages = {547-570},
    year  = {2018},
    publisher = {Taylor & Francis},
    doi = {10.1080/13658816.2017.1400548},
    URL = {
            https://doi.org/10.1080/13658816.2017.1400548
    },
    eprint = {
            https://doi.org/10.1080/13658816.2017.1400548   
    }
    }
