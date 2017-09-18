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

mov ecx, [rdi + offset_N] ; r8d = N

inc rcx             ; rcx = N+1
mov r8, rcx         ; r8 = N+1
inc r8              ; r8 = N+2
shl r8, 4           ; r8 = (N+2)*16
lea r9, [rdi + r8]  ; r9 apunta al primero de la segunda fila
mov r11, r8         ; r10 = (N+2)*16
mul r8, rcx         ; r8 = (N+1)*(N+2)*16
lea r11, [rdi + r8] ; r11 apunta al primero de la última fila
dec rcx             ; rcx = N
mul r10, rcx        ; r10 apunta al primero de la anteúltima fila


lea r9, [rdi + r8] ; r9 apunta al primero de la última fila
mov r8, rdi        ; r8 apunta al primero de la primera fila 


; r8 primera fila     ; rdi
; r9 segunda fila     ; rdi + 16*(N+2)
; r10 anteúltima fila ; rdi + 16*(N+2)*(N)
; r11 última fila     ; rdi + 16*(N+2)*(N+1)


shr ecx, 2 ; proceso de a 4 elementos

.ciclo:
  movups xmm0, [r8]
  
  add r8, 16
loop .ciclo

ret

global solver_project
solver_project:
; void solver_project ( fluid_solver* solver, float * p, float * div )
; rdi = fluid_solver* solver
; rsi = float* p
; rdx = float* div
ret