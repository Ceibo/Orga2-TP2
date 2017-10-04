#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "solver.h"
#include "rdtsc.h"

#define IXX(i,j) ((i)+(size+2)*(j))

char* archivoResultados  =  "resultados.csv";
uint32_t funcionATestear;
uint32_t repeticiones = 100;
uint32_t tamano = 512;
char* codigoATestear = "asm";

// [ATENCIÓN]
// Ir probando distintos valores
// (estos valores afectan a solver_lin_solve)
const float a = 1.0f; // 0.3f
const float c = 4.0f; // 2.8f

//char *c;

/*const uint32_t flush_cache_size = 2*1024*1024; // Allocate 2M. Set much larger then L2
void flush_cache() {
    for (uint32_t i = 0; i < 10; i++)
    	for (uint32_t j = 0; j < flush_cache_size; j++)
        	c[j] = i*j;
}*/

unsigned long solver_set_bnd_ticks_count(uint32_t size, uint32_t b) {
	fluid_solver* solver = solver_create(size, 0.05, 0, 0);
	solver_set_initial_velocity(solver);

	unsigned long start, end;
  if (strcmp(codigoATestear, "asm") == 0) {
  	RDTSC_START(start);
  	solver_set_bnd(solver, b, solver->v);
  	RDTSC_STOP(end);
  } else {
    RDTSC_START(start);
    solver_set_bnd_c(solver, b, solver->v);
    RDTSC_STOP(end);
  }

	solver_destroy(solver);
	return end - start;
}

void test_solver_set_bnd() {
	size_t i;
	unsigned long* ticks = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones * 3);
	//c = (char *)malloc(flush_cache_size);
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks[i] = solver_set_bnd_ticks_count(tamano, 1);
	}
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks[i + repeticiones] = solver_set_bnd_ticks_count(tamano, 2);
	}
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks[i + 2*repeticiones] = solver_set_bnd_ticks_count(tamano, 3);
	}
	printf("%lu", ticks[0]);
	for (i = 1; i < repeticiones*3; i++) {
		printf(",%lu", ticks[i]);	
	}
	free(ticks);
	//free(c);
}

unsigned long solver_project_ticks_count(uint32_t size, uint32_t b) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
  solver_set_bnd(solver, b, solver->u);
  solver_set_bnd(solver, b, solver->v);

  float* div = (float*) malloc(sizeof(float) * (size+2) * (size+2));
  float* p = (float*) malloc(sizeof(float) * (size+2) * (size+2));

  uint32_t i, j, k;

  // [ATENCIÓN]
  // Ir probando con distintos valores de llenado de la matriz
  k = 0;
  for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      div[IXX(j, i)] = k;
      p[IXX(i, j)] = k;
      k++;
    }
  }

  unsigned long start, end;
  if (strcmp(codigoATestear, "asm") == 0) {
    RDTSC_START(start);
    solver_project(solver, p, div);
    RDTSC_STOP(end);
  } else {
    RDTSC_START(start);
    solver_project_c(solver, p, div);
    RDTSC_STOP(end);
  }

  solver_destroy(solver);
  return end - start;
}

void test_solver_project() {
  size_t i;
  unsigned long* ticks = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones * 3);
  for (i = 0; i < repeticiones; ++i) {
    ticks[i] = solver_project_ticks_count(tamano, 1);
  }
  for (i = 0; i < repeticiones; ++i) {
    ticks[i + repeticiones] = solver_project_ticks_count(tamano, 2);
  }
  for (i = 0; i < repeticiones; ++i) {
    ticks[i + 2*repeticiones] = solver_project_ticks_count(tamano, 3);
  }
  printf("%lu", ticks[0]);
  for (i = 1; i < repeticiones*3; i++) {
    printf(",%lu", ticks[i]); 
  }
  free(ticks);
}

unsigned long solver_lin_solve_ticks_count(uint32_t size, uint32_t b, float a, float c) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
  solver_set_bnd(solver, b, solver->u);
  solver_set_bnd(solver, b, solver->v);

  unsigned long start, end;
  if (strcmp(codigoATestear, "asm") == 0) {
    RDTSC_START(start);
    solver_lin_solve(solver, b, solver->u, solver->v, a, c);
    RDTSC_STOP(end);
  } else {
    RDTSC_START(start);
    solver_lin_solve_c(solver, b, solver->u, solver->v, a, c);
    RDTSC_STOP(end);
  }

  solver_destroy(solver);
  return end - start;
}

void test_solver_lin_solve() {
  size_t i;
  unsigned long* ticks = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones * 3);
  for (i = 0; i < repeticiones; ++i) {
    ticks[i] = solver_lin_solve_ticks_count(tamano, 1, a, c);
  }
  for (i = 0; i < repeticiones; ++i) {
    ticks[i + repeticiones] = solver_lin_solve_ticks_count(tamano, 2, a, c);
  }
  for (i = 0; i < repeticiones; ++i) {
    ticks[i + 2*repeticiones] = solver_lin_solve_ticks_count(tamano, 3, a, c);
  }
  printf("%lu", ticks[0]);
  for (i = 1; i < repeticiones*3; i++) {
    printf(",%lu", ticks[i]); 
  }
  free(ticks);
}

void run_tests() {
	switch (funcionATestear){
		case 0:
			test_solver_set_bnd();
			break;
		case 1:
			test_solver_lin_solve();
			break;
		case 2:
			test_solver_project();
	}
}

int main (int argc, char ** argv) {
	if (argc < 2) {
		printf("Debe especificar la función a testear\n");
		exit(EXIT_FAILURE);
	} else if (strcmp(argv[1], "solver_lin_solve") == 0) {
		funcionATestear = 1;
	} else if (strcmp(argv[1], "solver_set_bnd") == 0) {
		funcionATestear = 0;
	} else if (strcmp(argv[1], "solver_project") == 0) {
		funcionATestear = 2;
	} else {
		printf("Nombre de función desconocido. Las opciones son: solver_lin_solve, solver_set_bnd, solver_project\n");
		exit(EXIT_FAILURE);
	}
	if (argc > 2) {
		archivoResultados = argv[2];
	}
	if (argc > 3) {
		repeticiones = atoi(argv[3]);
	}
  if (argc > 4) {
    tamano = atoi(argv[4]);
  }
  if (argc > 5) {
    codigoATestear = argv[4];
    if (strcmp(codigoATestear, "asm") != 0 && strcmp(codigoATestear, "c") != 0) {
      printf("Lenguaje a testear desconocido. Las opciones son: c, asm");
      exit(EXIT_FAILURE);
    }
  }
	int save_out = dup(1);
	remove(archivoResultados);
	int pFile = open(archivoResultados, O_RDWR|O_CREAT|O_APPEND, 0600);
	if (-1 == dup2(pFile, 1)) {
		printf("cannot redirect stdout");
		exit(EXIT_FAILURE);
	}
	run_tests();
	fflush(stdout);
	close(pFile);
	dup2(save_out, 1);
	exit(EXIT_SUCCESS);  
}


