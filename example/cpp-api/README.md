### Use fmm in an external project

Build the program

```
mkdir build
cd build
cmake .. -DFMM_LIB -DFMM_HEADER
make
sudo make install
```

Run it with

```
fmm_print_network example/data/edges.shp
```
