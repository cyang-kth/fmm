### OpenMP extension of map matching

The output of parallel rtree is in the log file.

### Verification of the result

Find the lines only in old.txt

    comm -23 <(sort old.txt) <(sort  new.txt)
    comm -23 <(sort ubodt1000.txt) <(sort ubodt1000_omp.txt) 

### About OpenMP

    #pragma omp parallel
    {
      code1();
    #pragma omp for
      for (i=1; i<=4*N; i++) {
        code2();
      }
      code3();
    }

The parallel execution of a loop can be handled:

1. Adjust bounds in each thread
2. `parallel for` pragmas do **not create** a team of threads: they take the team of threads that is **active**, and divide the loop iterations over them. It implies that `for` needs to be placed inside `parallel` region.
3. A parallel loop is an example of independent work units that are numbered.
4. The \indexpragmadef{single} and \indexpragmadef{master} pragma limit the execution of a block to a single thread. This can for instance be used to print tracing information or doing I/O operations.
5. `private` keyword is added to simulate local variable because scope concept is not available in Fortan. 

Functions that are called from a parallel region fall in the dynamic scope of that parallel region. The rules for variables in that function are as follows:
- Any variables locally defined to the function are **private**.
- static variables in C and save variables in Fortran are shared.
- The function arguments inherit their status from the calling environment.

The private directive declares data to have a separate copy in the memory of each thread. Such private variables are initialized as they would be in a main program. Any computed value goes away at the end of the parallel region. (However, see below.) Thus, you should not rely on any initial value, or on the value of the outer variable after the region.

### Synchronization

A barrier defines a point in the code where **all active threads will stop until all threads have arrived at that point**. With this, you can guarantee that certain calculations are finished. For instance, in this code snippet, computation of  y can not proceed until another thread has computed its value of x.

Apart from the barrier directive, which inserts an explicit barrier, OpenMP has implicit barriers after a load sharing construct.(One for pragma is finished before another) Therefore, **there is an implicit barrier at the end of a parallel region**.





#### Default

- Loop variables in an omp for are private;
- Local variables in the parallel region are private.
- Shared object: each thread gets a private pointer, but all pointers point to the same object.

#### False sharing

While this code is correct, it may be inefficient because of a phenomemon called false sharing . Even though the threads write to separate variables, those variables are likely to be on the same cacheline (see \HPSCref{sec:falseshare} for an explanation). This means that the cores will be wasting a lot of time and bandwidth updating each other's copy of this cacheline.

To **store data for each thread** is likely to suffer from this problem. 

- http://pages.tacc.utexas.edu/~eijkhout/pcse/html/omp-loop.html
- https://stackoverflow.com/questions/19278435/avoiding-false-sharing-in-openmp-with-arrays

