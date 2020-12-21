### Use fmm in an external project

First install fmm with the option `FMM_INSTALL_HEADER`

```
# In the project folder of fmm
cd build
cmake .. -DFMM_INSTALL_HEADER
make
sudo make install
```

In the current folder, build the program
```
mkdir build
cd build
cmake ..
make
sudo make install
```

Run it with

```
fmm_print_network example/data/edges.shp
```

You should see output like

```
[2020-12-04 13:50:29.350] [info] [main.cpp:9] Use FMM in an external project
[2020-12-04 13:50:29.350] [info] [main.cpp:14] Network file example/data/edges.shp
[2020-12-04 13:50:29.351] [info] [network.cpp:72] Read network from file example/data/edges.shp
[2020-12-04 13:50:29.356] [info] [network.cpp:166] Number of edges 30 nodes 17
[2020-12-04 13:50:29.356] [info] [network.cpp:168] Field index: id 1 source 2 target 3
[2020-12-04 13:50:29.356] [info] [network.cpp:170] Read network done.
[2020-12-04 13:50:29.356] [info] [main.cpp:16] Network node count 17
[2020-12-04 13:50:29.356] [info] [main.cpp:17] Network edge count 30
```
