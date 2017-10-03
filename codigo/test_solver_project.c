#include "solver.h"
#include "assert.h"


#define IXX(i,j) ((i)+(size+2)*(j))

// Constantes
const long double diferencia_maxima_permitida_en_comparaciones = 0;

void test_solver_project(uint32_t size, uint32_t b) {
  // Configuración inicial
  fluid_solver* solver_c = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver_c);
  solver_set_bnd(solver_c, b, solver_c->u);
  solver_set_bnd(solver_c, b, solver_c->v);

  float* div_c = (float*) malloc(sizeof(float) * (size+2) * (size+2));
  float* p_c = (float*) malloc(sizeof(float) * (size+2) * (size+2));

  fluid_solver* solver_asm = solver_create(size, 0.05, 0, 0);
  solver_set_initial_velocity(solver_asm);
  solver_set_bnd(solver_asm, b, solver_asm->u);
  solver_set_bnd(solver_asm, b, solver_asm->v);

  float* div_asm = (float*) malloc(sizeof(float) * (size+2) * (size+2));
  float* p_asm = (float*) malloc(sizeof(float) * (size+2) * (size+2));

  uint32_t i, j, k;

  for (i = 0, k = 1; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j, ++k) {
      div_c[IXX(j, i)] = p_c[IXX(i, j)] = div_asm[IXX(j, i)] = p_asm[IXX(i, j)] = k;
    }
  }

  // Pruebas
  solver_project_c(solver_c, p_c, div_c);
  solver_project(solver_asm, p_asm, div_asm);

  float max_dif = 0;
  float aux;

  for (i = 0; i < (size+2); ++i) {
    for (j = 0; j < (size+2); ++j) {
      assert((aux = fabs(p_c[IXX(i, j)] - p_asm[IXX(i, j)])) <= diferencia_maxima_permitida_en_comparaciones);
      max_dif = aux > max_dif ? aux : max_dif;
      assert((aux = fabs(div_c[IXX(i, j)] - div_asm[IXX(i, j)])) <= diferencia_maxima_permitida_en_comparaciones);
      max_dif = aux > max_dif ? aux : max_dif;
      assert((aux = fabs(solver_c->u[IXX(i, j)] - solver_asm->u[IXX(i, j)])) <= diferencia_maxima_permitida_en_comparaciones);
      max_dif = aux > max_dif ? aux : max_dif;
      assert((aux = fabs(solver_c->v[IXX(i, j)] - solver_asm->v[IXX(i, j)])) <= diferencia_maxima_permitida_en_comparaciones);
      max_dif = aux > max_dif ? aux : max_dif;
    }
  }

  printf("Tamaño de la matriz: %i. La diferencia máxima es: %f\n", size, max_dif);

  // Limpieza
  solver_destroy(solver_c);
  solver_destroy(solver_asm);
  free(div_c);
  free(p_c);
  free(div_asm);
  free(p_asm);
}

int main() {
  printf("Máximo error permitido: %Lf\n", diferencia_maxima_permitida_en_comparaciones);
  test_solver_project(4, 1);
  test_solver_project(4, 2);
  test_solver_project(4, 3);
  test_solver_project(16, 1);
  test_solver_project(16, 2);
  test_solver_project(16, 3);
  test_solver_project(32, 1);
  test_solver_project(32, 2);
  test_solver_project(32, 3);
  test_solver_project(64, 1);
  test_solver_project(64, 2);
  test_solver_project(64, 3);
  test_solver_project(128, 1);
  test_solver_project(128, 2);
  test_solver_project(128, 3);
  test_solver_project(256, 1);
  test_solver_project(256, 2);
  test_solver_project(256, 3);
  test_solver_project(512, 1);
  test_solver_project(512, 2);
  test_solver_project(512, 3);
  printf("TEST APROBADO\n");
  return 0;
}