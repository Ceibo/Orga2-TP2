; fluid_solver
%define offset_N 0
%define offset_u 16
%define offset_v 24

section .text

global solver_lin_solve
solver_lin_solve:
; void solver_lin_solve ( fluid_solver* solver, uint32_t b, float * x, float * x0, float a, float c )
; rdi = fluid_solver* solver
; esi = uint32_t b
; rdx = float* x
; rcx = float* x0
; xmm0 = float a
; xmm1 = float b
ret

global solver_set_bnd
solver_set_bnd:
; void solver_set_bnd ( fluid_solver* solver, uint32_t b, float * x )
; rdi = fluid_solver* solver
; esi = uint32_t b
; rdx = float* x
ret

global solver_project
solver_project:
; void solver_project ( fluid_solver* solver, float * p, float * div )
; rdi = fluid_solver* solver
; rsi = float* p
; rdx = float* div
ret