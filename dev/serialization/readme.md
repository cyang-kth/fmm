### UBODT serialization with boost library

Run `ubodt2binary` with

    make
    # Convert txt to binary
    ubodt2binary ubodt.txt ubodt.bin
    # Convert binary to txt
    binary2ubodt ubodt.binary temp.txt
    # Check result
    md5sum *.txt

The output is

    12910e34803e0b3791776e2dd574e572  test.txt
    12910e34803e0b3791776e2dd574e572  ubodt.txt

### Compression test

The statistics are collected from converting a large UBODT file in CSV format (**5 million**) rows. 

| Format          | size |
|-----------------|------|
| CSV             | 193M |
| hdf5 (pandas)   | 172M |
| Pickle (pandas) | 115M |
| Binary (boost)  | 115M |

### Measurement of the binary result

format;time;size(MB)
binary;229.569;508
csv;274.379;766

