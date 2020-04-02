<div align="center">
  <img src="img/fmm_social.jpg">
</div>

**`FMM-Wiki`** |
------------------- |
[![Wiki](https://img.shields.io/badge/link-wiki-blue.svg)](https://fmm-wiki.github.io/) |



FMM is an open source map matching framework designed for maximizing performance, scalability and functionality. 

### Why should I use FMM?

- High performance: C++ implementation with Rtree for spatial search, optimized routing, parallel computing. 
- Python API supported: run map matching in jupyter-notebook and web demo
- Scalibility: it has been tested on matching **millions of GPS points** to **country level** road network containing **millions of road edges**.  
- Multiple input data format support: 
  - Road network: ESRI shapefile or Geopackage，OSM can also be supported with [manual preprocessing](https://github.com/cyang-kth/osm_mapmatching). 
  - GPS: CSV point, CSV Trajectory or Shapefile trajectory.
- Customized output format: road edge matched to each point, observation error, distance traveled. 
- Multiple map matching algorithms implemented: 
  - FMM: suitable for small road network and dense GPS data
  - STMatch: suitable for large road network with sparse GPS data
- Cross platform support: Unix, Mac and Windows.  

We encourage contribution with feature request, bug report or developping new map matching algorithms using the framework. 

### Demo

Interactive web demo

<img src="img/demo3.gif" width="400"/> <img src="img/demo4.gif" width="400"/>

Jupyter-notebook  



### Installation, examples and documentation.

Check the [https://fmm-wiki.github.io/](https://fmm-wiki.github.io/) for details.

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
