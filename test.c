#include "solver.h"
#include "assert.h"

void test_solver_set_bnd(uint32_t size, uint32_t b) {
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_density(solver);

  float* matrix = (float*) malloc(sizeof(float) * (size+2) * (size+2));

  uint32_t i, j, k;

  for (i = 0, k = 1; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j, ++k) {
      matrix[IX(j, i)] = solver->u[IX(j, i)] = k;
    }
  }

  printf("Initial values\n");
  for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      printf("%2.2f ", matrix[IX(j, i)]);
    }
    printf("\n");
  }

  solver_set_bnd_c(solver, b, matrix);

  printf("Final values\n");
  for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      printf("%2.2f ", matrix[IX(j, i)]);
    }
    printf("\n");
  }

  solver_set_bnd(solver, b, solver->u);

  for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      assert(matrix[IX(i, j)] == solver->u[IX(i, j)]);
    }
  }

  solver_destroy(solver);
  free(matrix);
}

/*void mostrar_cosas() {
  fluid_solver* solver = solver_create(4, 0.05, 0, 0);
  solver_set_initial_density(solver);
  solver_set_initial_velocity(solver);

  printf("Initial density\n");
  for (uint32_t i = 0; i < solver->N+2; ++i) {
    for (uint32_t j = 0; j < solver->N+2; ++j) {
      printf("%.2f ", solver->u[IX(i, j)]);
    }
    printf("\n");
  }

  printf("\nInitial velocity\n");
  for (uint32_t i = 0; i < solver->N+2; ++i) {
    for (uint32_t j = 0; j < solver->N+2; ++j) {
      printf("%.2f ", solver->v[IX(i, j)]);
    }
    printf("\n");
  }

  solver_set_bnd(solver)
}*/

int main() {
  test_solver_set_bnd(4, 3);
  return 0;
}