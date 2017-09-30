#include "solver.h"
#include "assert.h"

// Constantes
const long double diferencia_maxima_permitida_en_comparaciones = 0.0001l;

void test_solver_lin_solve(uint32_t size, uint32_t b, float a, float c) {
  // Configuración inicial
  fluid_solver* solver = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver);
  solver_set_bnd(solver, b, solver->u);
  solver_set_bnd(solver, b, solver->v);

  // Copias de las matrices para testear
  size += 2;
  float* u = (float*) malloc(sizeof(float) * size * size);
  float* v = (float*) malloc(sizeof(float) * size * size);
  uint32_t i, j;
  for (i = 0; i < size; ++i) {
    for (j = 0; j < size; ++j) {
      u[IX(j, i)] = solver->u[IX(j, i)];
      v[IX(j, i)] = solver->v[IX(j, i)];
    }
  }

  // Pruebas
  solver_lin_solve_c(solver, b, solver->u, solver->v, a, c);
  solver_lin_solve(solver, b, u, v, a, c);
  for (i = 0; i < size; ++i) {
    for (j = 0; j < size; ++j) {
      assert(fabs(u[IX(i, j)] - solver->u[IX(i, j)]) < diferencia_maxima_permitida_en_comparaciones);
      assert(fabs(v[IX(i, j)] - solver->v[IX(i, j)]) < diferencia_maxima_permitida_en_comparaciones);
    }
  }

  // Limpieza
  solver_destroy(solver);
  free(u);
  free(v);
}

int main() {
  test_solver_lin_solve(4, 0, 0.3f, 2.8f);
  test_solver_lin_solve(4, 1, 0.3f, 2.8f);
  test_solver_lin_solve(4, 2, 0.3f, 2.8f);
  test_solver_lin_solve(16, 0, 0.3f, 2.8f);
  test_solver_lin_solve(16, 1, 0.3f, 2.8f);
  test_solver_lin_solve(16, 2, 0.3f, 2.8f);
  test_solver_lin_solve(32, 0, 0.3f, 2.8f);
  test_solver_lin_solve(32, 1, 0.3f, 2.8f);
  test_solver_lin_solve(32, 2, 0.3f, 2.8f);
  test_solver_lin_solve(64, 0, 0.3f, 2.8f);
  test_solver_lin_solve(64, 1, 0.3f, 2.8f);
  test_solver_lin_solve(64, 2, 0.3f, 2.8f);
  test_solver_lin_solve(128, 0, 0.3f, 2.8f);
  test_solver_lin_solve(128, 1, 0.3f, 2.8f);
  test_solver_lin_solve(128, 2, 0.3f, 2.8f);
  test_solver_lin_solve(256, 0, 0.3f, 2.8f);
  test_solver_lin_solve(256, 1, 0.3f, 2.8f);
  test_solver_lin_solve(256, 2, 0.3f, 2.8f);
  test_solver_lin_solve(512, 0, 0.3f, 2.8f);
  test_solver_lin_solve(512, 1, 0.3f, 2.8f);
  test_solver_lin_solve(512, 2, 0.3f, 2.8f);

  test_solver_lin_solve(4, 0, 1.0f, 4.0f);
  test_solver_lin_solve(4, 1, 1.0f, 4.0f);
  test_solver_lin_solve(4, 2, 1.0f, 4.0f);
  test_solver_lin_solve(16, 0, 1.0f, 4.0f);
  test_solver_lin_solve(16, 1, 1.0f, 4.0f);
  test_solver_lin_solve(16, 2, 1.0f, 4.0f);
  test_solver_lin_solve(32, 0, 1.0f, 4.0f);
  test_solver_lin_solve(32, 1, 1.0f, 4.0f);
  test_solver_lin_solve(32, 2, 1.0f, 4.0f);
  test_solver_lin_solve(64, 0, 1.0f, 4.0f);
  test_solver_lin_solve(64, 1, 1.0f, 4.0f);
  test_solver_lin_solve(64, 2, 1.0f, 4.0f);
  test_solver_lin_solve(128, 0, 1.0f, 4.0f);
  test_solver_lin_solve(128, 1, 1.0f, 4.0f);
  test_solver_lin_solve(128, 2, 1.0f, 4.0f);
  test_solver_lin_solve(256, 0, 1.0f, 4.0f);
  test_solver_lin_solve(256, 1, 1.0f, 4.0f);
  test_solver_lin_solve(256, 2, 1.0f, 4.0f);
  test_solver_lin_solve(512, 0, 1.0f, 4.0f);
  test_solver_lin_solve(512, 1, 1.0f, 4.0f);
  test_solver_lin_solve(512, 2, 1.0f, 4.0f);
  return 0;
}