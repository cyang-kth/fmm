all:hello hello_thread
hello:
	gcc -fopenmp hello.c -o hello.exe
hello_thread:
	gcc -fopenmp hello_thread.c -o hello_thread.exe
rtree_example:
	g++ -std=gnu++11 -O3 rtree_example.cpp -o rtree_example.exe
parallel_rtree:
	g++ -std=gnu++11 -O3 -fopenmp parallel_rtree.cpp -o parallel_rtree.exe