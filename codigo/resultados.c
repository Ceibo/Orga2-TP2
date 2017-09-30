#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "solver.h"
#include "rdtsc.h"

char *archivoResultados  =  "resultados.txt";
char *c;

unsigned long solver_set_bnd_ticks_count(uint32_t size, uint32_t b) {
	fluid_solver* solver = solver_create(size, 0.05, 0, 0);
	solver_set_initial_velocity(solver);

	unsigned long start, end;
	RDTSC_START(start);
	
	solver_set_bnd(solver, b, solver->v);

	RDTSC_STOP(end);
	solver_destroy(solver);
	return end - start;
}

const int flush_cache_size = 2*1024*1024; // Allocate 2M. Set much larger then L2
void flush_cache() {
    for (int i = 0; i < 0xffff; i++)
    	for (int j = 0; j < flush_cache_size; j++)
        	c[j] = i*j;
}

void run() {
	size_t i;
	c = (char *)malloc(flush_cache_size);
	for (i = 0; i < 100; ++i) {
		//flush_cache();
		printf("%lu\n", solver_set_bnd_ticks_count(16, 1));
	}
	for (i = 0; i < 100; ++i) {
		//flush_cache();
		printf("%lu\n", solver_set_bnd_ticks_count(16, 2));
	}
	for (i = 0; i < 100; ++i) {
		//flush_cache();
		printf("%lu\n", solver_set_bnd_ticks_count(16, 3));
	}
	free(c);
}

int main (void) {
	//run();
	int save_out = dup(1);
	remove(archivoResultados);
	int pFile = open(archivoResultados, O_RDWR|O_CREAT|O_APPEND, 0600);
	if (-1 == dup2(pFile, 1)) { perror("cannot redirect stdout"); return 255; }
	run();
	fflush(stdout);
	close(pFile);
	dup2(save_out, 1);
	return 0;    
}


