### Command line examples

:warning: For `fmm`, it takes an ubodt file that can be generated using the `ubodt_gen`.

- Precompute UBODT

  ```bash
  # XML configuration
  ubodt_gen ubodt_config.xml
  # Command line arguments
  ubodt_gen --network ../data/edges.shp --output ../data/ubodt.txt --delta 3
  ```

- Precompute UBODT parallelly

  ```bash
  # XML configuration
  ubodt_gen ubodt_config_omp.xml
  # Command line arguments
  ubodt_gen --network ../data/edges.shp --output ../data/ubodt.txt --delta 3 --use_omp
  ```

- Matching GPS trajectory in shapefile using fmm

  ```bash
  # XML configuration
  fmm fmm_config.xml
  # Command line arguments
  fmm --ubodt ../data/ubodt.txt --network ../data/edges.shp --gps ../data/trips.shp -k 4 -r 0.4 -e 0.5 --output mr.txt
  ```

- Matching GPS trajectory in shapefile using stmatch

  ```bash
  # XML configuration
  # XML configuration
  stmatch stmatch_config.xml
  # Command line arguments
  stmatch --network ../data/edges.shp --gps ../data/trips.shp -k 4 -r 0.4 -e 0.5 --output mr.txt
  ```

- Matching GPS trajectory in CSV file using fmm

  ```bash
  # XML configuration
  fmm fmm_config_csv_trajectory.xml
  # Command line arguments
  fmm --ubodt ../data/ubodt.txt --network ../data/edges.shp --gps ../data/trips.csv -k 4 -r 0.4 -e 0.5 --output mr.txt
  ```

- Matching GPS Points in CSV file using fmm  

  ```bash
  # XML configuration
  fmm fmm_config_csv_point.xml
  # Command line arguments
  fmm --ubodt ../data/ubodt.txt --network ../data/edges.shp --gps ../data/gps.csv --gps_point -k 4 -r 0.4 -e 0.5 --output mr.txt
  ```

- Parallel map matching

  ```bash
  # XML configuration
  fmm fmm_config_omp.xml
  # Command line arguments
  fmm --ubodt ../data/ubodt.txt --network ../data/edges.shp --gps ../data/gps.csv --gps_point -k 4 -r 0.4 -e 0.5 --output mr.txt --use_omp
  ```

- Customized output fields
  ```bash
  # XML configuration
  fmm fmm_config_output_fields.xml
  # Command line arguments
  fmm --ubodt ../data/ubodt.txt --network ../data/edges.shp --gps ../data/gps.csv --gps_point -k 4 -r 0.4 -e 0.5 --output mr.txt --output_fields opath,cpath,mgeom,tpath,spdist
  ```
