### OpenMP extension of map matching

The output of parallel rtree is in the log file.

### Verification of the result

Find the lines only in old.txt

    comm -23 <(sort old.txt) <(sort  new.txt)


