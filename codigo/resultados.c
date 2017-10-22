#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include "solver.h"
#include "rdtsc.h"

//#include <stdio.h>
#include <string.h>

#define IXX(i,j) ((i)+(size+2)*(j))

char* mediciones16  =  "_mediciones_tam_16.csv";
char* mediciones32  =  "_mediciones_tam_32.csv";
char* mediciones64  =  "_mediciones_tam_64.csv";
char* mediciones128  =  "_mediciones_tam_128.csv";
char* mediciones256  =  "_mediciones_tam_256.csv";
char* mediciones512  =  "_mediciones_tam_512.csv";  

uint32_t funcionATestear;
uint32_t repeticiones = 100;
uint32_t tamano = 0; //16, 32, 64, 128, 256, 512
 
//****************** parAmetros solver lin solve  ***********************//

// a,b,c[0] : op1,      a,b,c[1] : op2 ,       a,b,c[2] : op3,         a,b,c[3]: op4

float a[] = { 1.0f,0.3f, 100.0f, -10.0f };
float c[] = {4.0f,2.8f,  20.0f, 0.02f};
uint32_t b[] = { 1,2, 3,  10};

//****************** parAmetros solver set bnd  ***********************//

// b[0] : op1,      b[1] : op2 ,        b[2] : op3,         b[3]: op4
 
//uint32_t b[] = {1,2,3,10};


//****************** parAmetros solver project  ***********************//

// k,h[0] : op1,      k,h[1] : op2 ,       k,h[2] : op3,         k,h[3]: op4
 
float k[] = { 0.1f,0.2f, -10.0f, 1000.0f};
float h[] = { 0.2f,-100.0f, 0.08f, 2000.0f};
 

/*const uint32_t flush_cache_size = 2*1024*1024; // Allocate 2M. Set much larger then L2
void flush_cache() {
    for (uint32_t i = 0; i < 10; i++)
    	for (uint32_t j = 0; j < flush_cache_size; j++)
        	c[j] = i*j;
}*/


 
unsigned long solver_set_bnd_ticks_count_asm(uint32_t size, int caso) {
	fluid_solver* solver = solver_create(size, 0.05, 0, 0);
	solver_set_initial_velocity(solver);
	solver_set_initial_density(solver);
	unsigned long start, end;
	RDTSC_START(start);
	
	solver_set_bnd(solver, b[caso], solver->v);// versiOn asm 

	RDTSC_STOP(end);
	solver_destroy(solver);
	return end - start;
}


unsigned long solver_set_bnd_ticks_count_c(uint32_t size, int caso) {
	fluid_solver* solver = solver_create(size, 0.05, 0, 0);
	solver_set_initial_velocity(solver);
    solver_set_initial_density(solver);
	unsigned long start, end;
	RDTSC_START(start);
	
	solver_set_bnd_c(solver, b[caso], solver->v);// versiOn c

	RDTSC_STOP(end);
	solver_destroy(solver);
	return end - start;
}

unsigned long solver_lin_solve_ticks_count_c(uint32_t size,  int caso) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver); 
  solver_set_initial_density(solver);
  unsigned long start, end;
  
    RDTSC_START(start);
    solver_lin_solve_c(solver, b[caso], solver->u, solver->v, a[caso], c[caso]); //versiOn c
    RDTSC_STOP(end);
  

  solver_destroy(solver);
  return end - start;
}

unsigned long solver_lin_solve_ticks_count_asm(uint32_t size, int caso) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
  solver_set_initial_density(solver);

  unsigned long start, end;
  
    RDTSC_START(start);
    solver_lin_solve(solver, b[caso], solver->u, solver->v, a[caso], c[caso]); //versiOn asm
    RDTSC_STOP(end);
  

  solver_destroy(solver);
  return end - start;
}

      //*****************************************************************************//

unsigned long solver_project_ticks_count_c(uint32_t size,int caso) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
    solver_set_initial_density(solver);

  float* div = (float*) malloc(sizeof(float) * (size+2) * (size+2));
  float* q = (float*) malloc(sizeof(float) * (size+2) * (size+2));

  uint32_t i, j;
   
   float o = k[caso];
   float p = h[caso];
  // [ATENCIÓN]
  // Ir probando con distintos valores de llenado de la matriz, 
  //                                         opciones matriz
   for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      div[IXX(j, i)] = o ;
      q[IXX(i, j)] = p;
      o++;
      p++;
    }
  }
/* */
  unsigned long start, end;
  
    RDTSC_START(start);
    solver_project_c(solver, q, div);
    RDTSC_STOP(end);
  

  solver_destroy(solver);
  return end - start;
}

      //************************************************************************************//

unsigned long solver_project_ticks_count_asm(uint32_t size, int caso) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
    solver_set_initial_density(solver);

  float* div = (float*) malloc(sizeof(float) * (size+2) * (size+2));
  float* q = (float*) malloc(sizeof(float) * (size+2) * (size+2));

  uint32_t i, j;
   
  
   float o = k[caso];
   float p = h[caso];
  // [ATENCIÓN]
  // Ir probando con distintos valores de llenado de la matriz, 
  //                                         opciones matriz
   for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      div[IXX(j, i)] = o ;
      q[IXX(i, j)] = p;
      o++;
      p++;
    }
  }
/* */
  unsigned long start, end;
  
    RDTSC_START(start);
    solver_project(solver, q, div);
    RDTSC_STOP(end);
  

  solver_destroy(solver);
  return end - start;
}

//***************************************************************************************//
//****************** por cada test genera 2 tiradas: 100 para c y 100 asm *****************//
//****************************************************************************************//

void test_solver_project(uint32_t tamano,int caso) {
  size_t i;
  unsigned long* ticks_asm = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );
  	unsigned long* ticks_c = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );

  for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks_c[i] = solver_project_ticks_count_c(tamano, caso);
	}
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks_asm[i] = solver_project_ticks_count_asm(tamano,caso);
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

void test_solver_set_bnd(uint32_t tamano, int caso) {
	size_t i;
	unsigned long* ticks_asm = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );
	unsigned long* ticks_c = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );

	//c = (char *)malloc(flush_cache_size);
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks_c[i] = solver_set_bnd_ticks_count_c(tamano,caso);
	}
	for (i = 0; i < repeticiones; ++i) {
		//flush_cache();
		ticks_asm[i] = solver_set_bnd_ticks_count_asm(tamano, caso);
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
	//free(c);
}

void test_solver_lin_solve(uint32_t tamano, int caso) {
  size_t i;
  unsigned long* ticks_c = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );
  unsigned long* ticks_asm = (unsigned long*) malloc(sizeof(unsigned long) * repeticiones );

  for (i = 0; i < repeticiones; ++i) {
    ticks_c[i] = solver_lin_solve_ticks_count_c(tamano, caso);// 2do parAmetro: 1,2,3,10
  }
  for (i = 0; i < repeticiones; ++i) {
    ticks_asm[i] = solver_lin_solve_ticks_count_asm(tamano, caso);//2do parAmetro: 1,2,3,10
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

void run_tests(uint32_t tamano, int caso) {
	switch (funcionATestear){
		case 0:
			test_solver_set_bnd(tamano, caso);
			break;
		case 1:
			test_solver_lin_solve(tamano,caso);
			break;
		case 2:
			test_solver_project(tamano,caso);
	}
}

 //especificar como 2do parAmetro funciOn, 3er parAmetro a tamanio y 4to parAmetro a caso
//************** ej:  ./resultados solver_set_bnd tamanios_full casos_full *****************
//tamanios_full significa todos los tamanios a evaluar, caso contrario se evalUa tamanios: 16 y 512
// casos_full significa evaluar todos los casos , caso contrario se evalUa un solo caso: 1er caso

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
	
	uint32_t tamanios[] = {16, 512, 32, 64, 128, 256};
	char * nombres[] = {mediciones16,mediciones512,mediciones32,mediciones64,mediciones128,mediciones256};
	char * casos[] = {"_op1_","_op2_","_op3_","_op4_"};
	char  alias [50];
	char  aliasOp [50];
	char aliasNom [50]; 
	char * archivoName;
	uint32_t cantidad_full = sizeof(tamanios)/sizeof(tamanios[0]);
	uint32_t cantidad_simple = 2;
	uint32_t cantidad = 0;
	int casos_full = sizeof(casos)/sizeof(casos[0]);
	int casos_simple = 1;
	int argm = 0;
	if(argc > 3){
		if(strcmp(argv[2],"tamanios_full") == 0){
			cantidad = cantidad_full;
		}else{
			cantidad = cantidad_simple;
		}
		for(uint32_t i = 0; i < cantidad; i++){
		 
			strcpy(aliasNom,nombres[i]);
		   
			if(	strcmp(argv[3],"casos_full") == 0){
				argm = casos_full;
			}else{
				argm = casos_simple; 
		    }
			//repeticiones = atoi(argv[3]);
				for(int indiceCasos = 0; indiceCasos < argm;indiceCasos++){
					int save_out = dup(1);
					strcpy(alias, argv[1]);
					strcpy(aliasOp,casos[indiceCasos]);	
					archivoName = strcat(aliasOp,aliasNom);

					archivoName = strcat( alias, archivoName);
					remove(archivoName);
		
					int pFile = open(archivoName, O_RDWR|O_CREAT|O_APPEND, 0600);
					if (-1 == dup2(pFile, 1)) {
						printf("cannot redirect stdout");
						exit(EXIT_FAILURE);
					}
					run_tests(tamanios[i],indiceCasos);//indiceCasos es int
					fflush(stdout);
					close(pFile);
					dup2(save_out, 1);
				}
		//exit(EXIT_SUCCESS);  
	      
        
      
		}
  }
} 
