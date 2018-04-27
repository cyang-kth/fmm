#include <stdio.h>
#include <omp.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char *argv[]) {
	int num_vertices = 400;
	int k = 5;
	std::ofstream ofs("test.txt");
	int duplicates = num_vertices/k;
	std::vector<int> data(num_vertices);
	int progress = 0;
	// vector is like 000...111...222...333...444...
	for (int i = 0; i < num_vertices; ++i) {
		data[i] = i/duplicates;
	}
	#pragma omp parallel
	{
		// Initialization
		// printf("Thread in parallel %d\n", omp_get_thread_num());
		// int tid = omp_get_thread_num();
		int progress = 0;
		std::vector<int> count(k);
		std::stringstream buf;
		// Position 1
		// std::stringstream node_output_buf;
		#pragma omp for
		for (int source = 0; source < num_vertices; ++source) {
			int m = 0; 
			while (m<=source) {
				if (count.size()<data[m]){
					
				}
				count[data[m]]+=1;
				++m;
			}
			buf<<"Position "<<source<<" count ";
			for (int i=0;i<k;++i){
				buf<<count[i]<<" ";
			}
			buf<<"\n";
			// Clean the count result
			count.clear();
		}
		#pragma omp critical
		ofs << buf.rdbuf();
	}
	ofs.close();
};
