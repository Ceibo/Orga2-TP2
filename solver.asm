; fluid_solver
%define offset_N 0
%define offset_u 16
%define offset_v 24
%define cell_size 16

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

push r12
push r13
push r14

mov ecx, [rdi + offset_N] ; rcx = N

mov 

mov r8, rcx
add r8, 2 ; r8 = N+2
mov r14, r8

lea r9, [rdi + cell_size*r8]

mul r8, rcx ; r8 = (N+2)*N

lea r10, [rdi + cell_size*r8]

add r8, rcx
add r8, 2 ; r8 = (N+2)*(N+1)

lea r11, [rdi + cell_size*r8]

lea r8, [rdi + 16]

mov r13, r8
add r13, r14 ; apunta al anteúltimo de la segunda fila
mov r12, r9  ; apunta al primero de la segunda fila

; r8 primera fila     ; rdi + 16
; r9 segunda fila     ; rdi + 16*(N+2)
; r10 anteúltima fila ; rdi + 16*(N+2)*(N)
; r11 última fila     ; rdi + 16*(N+2)*(N+1)

shr ecx, 2

.ciclo:
  movups xmm0, [r9]
  movups xmm1, [r10]
  cmp esi, 2
  jne .end_if_0
  	call change_sign_of_single_packed_xmm0_and_xmm1
  .end_if_0:
  movups [r8], xmm0
  movups [r11], xmm1


  
  add r8, 16
  add r9, 16
  add r10, 16
  add r11, 16
loop .ciclo

pop r14
pop r13
pop r12
ret

change_sign_of_single_packed_xmm0_and_xmm1:
; supuestamente 0x0 se interpreta como cero en float

movps xmm2, xmm0
pxor xmm0
subps xmm0, xmm2

movps xmm2, xmm1
pxor xmm1
subps xmm1, xmm2
ret

global solver_project
solver_project:
; void solver_project ( fluid_solver* solver, float * p, float * div )
; rdi = fluid_solver* solver
; rsi = float* p
; rdx = float* div
ret