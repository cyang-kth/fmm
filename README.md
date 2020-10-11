<div align="center">
  <img src="img/fmm_social.jpg">
</div>

| Linux / macOS | Windows | Wiki          | Docs        |
| ------------- | ------- | ------------- | ----------- |
| [![Build Status](https://travis-ci.org/cyang-kth/fmm.svg?branch=master)](https://travis-ci.org/github/cyang-kth/fmm) | [![Build status](https://ci.appveyor.com/api/projects/status/8qee5c8iay75j1am?svg=true)](https://ci.appveyor.com/project/cyang-kth/fmm) | [![Wiki](https://img.shields.io/badge/wiki-website-blue.svg)](https://fmm-wiki.github.io/) | [![Documentation](https://img.shields.io/badge/docs-doxygen-blue.svg)](https://cyang-kth.github.io/fmm/) |


FMM is an open source map matching framework in C++ and Python. It solves the problem of matching noisy GPS data to a road network. The design considers maximizing performance, scalability and functionality.

### Features

- **High performance**: C++ implementation using Rtree, optimized routing, parallel computing (OpenMP).
- **Python API**: [jupyter-notebook](example/notebook) and [web demo](example/web_demo)
- **Scalibility**: millions of GPS points and millions of road edges.  
- **Multiple data format**:
  - Road network in OpenStreetMap or ESRI shapefile.
  - GPS data in Point CSV, Trajectory CSV and Trajectory Shapefile ([more details](https://fmm-wiki.github.io/docs/documentation/input/#gps-data)).
- **Detailed matching information**: traversed path, geometry, individual matched edges, GPS error, etc. More information at [here](https://fmm-wiki.github.io/docs/documentation/output/).
- **Multiple algorithms**: [FMM](http://www.tandfonline.com/doi/full/10.1080/13658816.2017.1400548) (for small and middle scale network) and [STMatch](https://dl.acm.org/doi/abs/10.1145/1653771.1653820) (for large scale road network)
- **Platform support**: Unix (ubuntu) , Mac and Windows(cygwin environment).
- **Hexagon match**: :tada: Match to the uber's [h3](https://github.com/uber/h3) Hexagonal Hierarchical Geospatial Indexing System. Check the [demo](example/h3).

We encourage contribution with feature request, bug report or developping new map matching algorithms using the framework.

### Examples

- Map matching on [OSM network](example/osm_example) (left) and ESRI shapefile (right)

<img src="example/osm_example/result.png" width="400"/><img src="img/shp_example.png" width="400"/>

Note that although FMM can directly read OSM file as input, the original OSM file is common to contain poor topology information (https://github.com/cyang-kth/fmm/issues/99). Therefore, it is recommended to use OSMNX to download a routable shapefile for OSM. Check the [OSMNX example](example/osmnx_example).

- [Match to Hexagons](example/h3)

<img src="example/h3/h8.png" width = "390" />
<img src="example/h3/h8i.png" width = "390" />
<img src="example/h3/h9i.png" width = "390" />
<img src="example/h3/h7i.png" width = "390" />

- [Interactive web demo](example/web_demo)

<img src="img/demo3.gif" width="400"/> <img src="img/demo4.gif" width="400"/>

- [Command line examples](example/command_line_example)
- [Jupyter-notebook](example/notebook)
- [OSMNX example](example/osmnx_example)

### Installation, tutorial and API.

Check [https://fmm-wiki.github.io/](https://fmm-wiki.github.io/).

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
