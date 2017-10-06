#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "solver.h"
#include "rdtsc.h"

#define IXX(i,j) ((i)+(size+2)*(j))

char* archivoResultados  =  "resultados.txt";
uint32_t funcionATestear;
uint32_t repeticiones = 100;
uint32_t tamano = 256; //16, 32, 64, 128, 256, 512

//parAmetros solver lin solve , opciones
const float a = 1.0f;        //1.0f, 0.3f , -10.0f
const float c = 4.0f;        //4.0f, 2.8f , 0.02f

//char *c;

/*const uint32_t flush_cache_size = 2*1024*1024; // Allocate 2M. Set much larger then L2
void flush_cache() {
    for (uint32_t i = 0; i < 10; i++)
    	for (uint32_t j = 0; j < flush_cache_size; j++)
        	c[j] = i*j;
}*/

unsigned long solver_set_bnd_ticks_count_asm(uint32_t size, uint32_t b) {
	fluid_solver* solver = solver_create(size, 0.05, 0, 0);
	solver_set_initial_velocity(solver);
	solver_set_initial_density(solver);
	unsigned long start, end;
	RDTSC_START(start);
	
	solver_set_bnd(solver, b, solver->v);// versiOn asm 

	RDTSC_STOP(end);
	solver_destroy(solver);
	return end - start;
}


unsigned long solver_set_bnd_ticks_count_c(uint32_t size, uint32_t b) {
	fluid_solver* solver = solver_create(size, 0.05, 0, 0);
	solver_set_initial_velocity(solver);
    solver_set_initial_density(solver);
	unsigned long start, end;
	RDTSC_START(start);
	
	solver_set_bnd_c(solver, b, solver->v);// versiOn c

	RDTSC_STOP(end);
	solver_destroy(solver);
	return end - start;
}

unsigned long solver_lin_solve_ticks_count_c(uint32_t size, uint32_t b, float a, float c) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
  solver_set_initial_density(solver);
  unsigned long start, end;
  
    RDTSC_START(start);
    solver_lin_solve_c(solver, b, solver->u, solver->v, a, c); //versiOn c
    RDTSC_STOP(end);
  

  solver_destroy(solver);
  return end - start;
}

unsigned long solver_lin_solve_ticks_count_asm(uint32_t size, uint32_t b, float a, float c) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
  solver_set_initial_density(solver);

  unsigned long start, end;
  
    RDTSC_START(start);
    solver_lin_solve(solver, b, solver->u, solver->v, a, c); //versiOn c
    RDTSC_STOP(end);
  

  solver_destroy(solver);
  return end - start;
}

      //*****************************************************************************//

unsigned long solver_project_ticks_count_c(uint32_t size) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
    solver_set_initial_density(solver);

  float* div = (float*) malloc(sizeof(float) * (size+2) * (size+2));
  float* p = (float*) malloc(sizeof(float) * (size+2) * (size+2));

  uint32_t i, j;
  float k,h;

  // [ATENCIÓN]
  // Ir probando con distintos valores de llenado de la matriz, 
  //                                         opciones matriz
  k =0.1; h = 0.2  ; //k = 0.1,h =0.2 ; k = 0.09, h = -100 ; k = -10, h = 0.08 ; k = 1000, h = 2000
  for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      div[IXX(j, i)] = k;
      p[IXX(i, j)] = h;
      k++;
      h++;
    }
  }
/* */
  unsigned long start, end;
  
    RDTSC_START(start);
    solver_project_c(solver, p, div);
    RDTSC_STOP(end);
  

  solver_destroy(solver);
  return end - start;
}

      //************************************************************************************//

unsigned long solver_project_ticks_count_asm(uint32_t size) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
    solver_set_initial_density(solver);

  float* div = (float*) malloc(sizeof(float) * (size+2) * (size+2));
  float* p = (float*) malloc(sizeof(float) * (size+2) * (size+2));

  uint32_t i, j;
  float k,h;
  // [ATENCIÓN]
  // Ir probando con distintos valores de llenado de la matriz, 
  //                                         opciones matriz
  k =  0.1; h = 0.2;//k = 0.1,h =0.2 ; k = 0.09, h = -100 ; k = -10, h = 0.08 ; k = 1000, h = 2000
  for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      div[IXX(j, i)] = k;
      p[IXX(i, j)] = h;
      k++;
      h++;
    }
  }
/* */
  unsigned long start, end;
  
    RDTSC_START(start);
    solver_project(solver, p, div);
    RDTSC_STOP(end);
  

  solver_destroy(solver);
  return end - start;
}

//***************************************************************************************//
//****************** por cada test genera 2 tiradas: 100 para c y 100 asm *****************//
//****************************************************************************************//

void test_solver_project() {
  size_t i;
  unsigned long* ticks_asm = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );
  	unsigned long* ticks_c = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );

  for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks_c[i] = solver_project_ticks_count_c(tamano);
	}
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks_asm[i] = solver_project_ticks_count_asm(tamano);
	}
  
  //printeamos , ticks_c = resultados c, ticks_asm = resultados asm , cantidad resultados = 100
	printf("%s%s%s\n", "Repeticion,","c,","asm"); //agregar etiqueta : c, asm
	for (i = 0; i < repeticiones; i++) {
		printf("%ld",i+1);
		printf(",%lu,", ticks_c[i]);	
		printf("%lu\n", ticks_asm[i]);	
	}
	free(ticks_c);
    free(ticks_asm);
}

void test_solver_set_bnd() {
	size_t i;
	unsigned long* ticks_asm = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );
	unsigned long* ticks_c = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );

	//c = (char *)malloc(flush_cache_size);
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks_c[i] = solver_set_bnd_ticks_count_c(tamano, 2);
	}
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks_asm[i] = solver_set_bnd_ticks_count_asm(tamano, 2);
	}
	
	//printeamos , ticks_c = resultados c, ticks_asm = resultados asm , cantidad resultados = 100
	printf("%s%s%s\n", "N repeticion,","c,","asm"); //agregar etiqueta : c, asm
	for (i = 0; i < repeticiones; i++) {
		printf("%ld",i+1);
		printf(",%lu,", ticks_c[i]);	
		printf("%lu\n", ticks_asm[i]);	
	}
	free(ticks_c);
    free(ticks_asm);
	//free(c);
}

void test_solver_lin_solve() {
  size_t i;
  unsigned long* ticks_c = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );
  unsigned long* ticks_asm = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );

  for (i = 0; i < repeticiones; ++i) {
    ticks_c[i] = solver_lin_solve_ticks_count_c(tamano, 2, a, c);// 2do parAmetro: 1,2,3,10
  }
  for (i = 0; i < repeticiones; ++i) {
    ticks_asm[i] = solver_lin_solve_ticks_count_asm(tamano, 2, a, c);//2do parAmetro: 1,2,3,10
  }
  
  //printeamos , ticks_c = resultados c, ticks_asm = resultados asm , cantidad resultados = 100
	printf("%s%s%s\n", "Repeticion,","c,","asm"); //agregar etiqueta : c, asm
	for (i = 0; i < repeticiones; i++) {
		printf("%ld",i+1);
		printf(",%lu,", ticks_c[i]);	
		printf("%lu\n", ticks_asm[i]);	
	}
  free(ticks_c);
  free(ticks_asm);
}

//void test_solver_project() {}
//void test_solver_lin_solve() {}

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
		printf("Nombre de función desconocido, las opciones son: solver_lin_solve, solver_set_bnd, solver_project\n");
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


