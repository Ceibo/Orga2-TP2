; offsets fluid_solver
%define offset_N 0
%define offset_dt 4
%define offset_diff 8
%define offset_visc 12
%define offset_u 16
%define offset_v 24
%define offset_u_prev 32
%define offset_v_prev 40
%define offset_dens 48
%define offset_dens_prev 56

; constantes para SIMD
%define float_size 4
%define xmm_size 16

section .text

%include "solver_set_bnd_functions.inc"

;global solver_lin_solve
;solver_lin_solve:
; void solver_lin_solve ( fluid_solver* solver, uint32_t b, float * x, float * x0, float a, float c )
; rdi = fluid_solver* solver
; esi = uint32_t b
; rdx = float* x
; rcx = float* x0
; xmm0 = float a
; xmm1 = float b
;ret

global solver_set_bnd
solver_set_bnd:
; void solver_set_bnd ( fluid_solver* solver, uint32_t b, float * x )
; rdi = fluid_solver* solver
; esi = uint32_t b
; rdx = float* x

; Esta implementación asume que N es mayor o igual a 4 y que N es múltiplo de 4, de acuerdo a lo hablado con Emiliano Höß y Mariano Cerruti en la clase del martes 19 de septiembre.

push r12
push r13
push r14
push r15
push rbx

mov ecx, [rdi + offset_N] ; rcx = N

call setear_punteros_a_la_matriz

; r8 2da celda de la 1ra fila         ; rdx + 16
; r9 2da celda de la 2da fila         ; rdx + 16*(N+2)
; r10 2da celda de la anteúltima fila ; rdx + 16*(N+2)*(N)
; r11 2da celda de la última fila     ; rdx + 16*(N+2)*(N+1)
; r12 = r9, pero se va a usar para hacer el proceso vertical de la columna izquierda
; r13 anteúltima celda de la 2da fila, para hacer el proceso vertical de la columna derecha
; r14 tiene el tamaño de una fila para hacer saltos

shr ecx, 2 ; se divide por 4 para procesar de a 4 elementos

.ciclo:

  ; filas (utilizamos SIMD)

  call procesar_filas

  ; para las columnas usamos SIMD solo para el cambio de signo, pero la lectura es individual

  call procesar_columnas

loop .ciclo

; r8 última celda de la 1ra fila
; r9 última celda de la 2da fila
; r10 última celda de la anteúltima fila
; r11 última celda de la última fila
; r12 segunda celda de la última fila
; r13 anteúltima celda de la última fila
; r14 tiene el tamaño de una fila para hacer saltos

call procesar_esquinas

pop rbx
pop r15
pop r14
pop r13
pop r12
ret

;global solver_project
;solver_project:
;void solver_project ( fluid_solver* solver, float * p, float * div )
;rdi = fluid_solver* solver
;rsi = float* p
;rdx = float* div

;ciclo1 ACA HAY QUE USAR SIMD
;div[IX(i,j)] = -0.5f*(solver->u[IX(i+1,j)]-solver->u[IX(i-1,j)]+solver->v[IX(i,j+1)]-solver->v[IX(i,j-1)])/solver->N;
;p[IX(i,j)] = 0;


;Hay que poner los parametros en los registros para llamar correctamente a la funcion

;solver_set_bnd ( solver, 0, div )//solver = rdi, 0 = xmm0, div = xmm1
;mov rdi, SOLVER
;mov xmm0, 0
;mov xmm1, DIV (ver donde estaba cuando lo escriba)
;call set_solver_bnd

;Voy a llamarla de vuelta, aca tengo que acomodar de nuevo las cosas en los registros.
;solver_set_bnd ( solver, 0, p )
;mov rdi, SOLVER
;mov xmm0, 0
;mov xmm1, P
;call set_solver_bnd

;Ahora llamo a esta otra, de nuevo tengo que poner en orden los registros
;solver_lin_solve ( solver, 0, p, div, 1, 4 ) // solver = rdi, 0 = rsi, p = rdx, div = rcx, 1 = xmm0, 4 = xmm1
;mov rdi, SOLVER
;mov rsi, 0 // xor rsi, rsi
;mov rdx, p
;mov rcx, div
;mov xmm0, 1
;mov xmm1, 4
;call solver_lin_solve

;ciclo2 Y ACA TAMBIEN HAY QUE USAR SIMD

;ret


