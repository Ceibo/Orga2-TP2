; fluid_solver
%define offset_N 0
%define offset_u 16
%define offset_v 24
%define cell_size 4
%define window_size 4*cell_size

section .data

cuatro_floats_contiguos_de_valor_dos: DD 2.e1, 2.e1, 2.e1, 2.e1

section .text

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

shr ecx, 2 ; proceso de a 4 elementos

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

change_sign_of_single_packed_xmm0_and_xmm1:
; supuestamente 0x0 se interpreta como cero en float

movaps xmm2, xmm0
pxor xmm0, xmm0
subps xmm0, xmm2

movaps xmm2, xmm1
pxor xmm1, xmm1
subps xmm1, xmm2

ret

setear_punteros_a_la_matriz:

mov r8, rcx
add r8, 2 ; r8 = N+2
mov r14, r8
shl r14, 4 ; r14 = (N+2)*16

lea r9, [rdx + cell_size*r8]

mov eax, r8d 
mul cx
mov r8d, eax ; r8 = (N+2)*N

lea r10, [rdx + cell_size*r8]

add r8, rcx
add r8, 2 ; r8 = (N+2)*(N+1)

lea r11, [rdx + cell_size*r8]

lea r8, [rdx + cell_size]

lea r13, [r8 + cell_size*r14]
mov r12, r9  ; apunta al primero de la segunda fila

ret

procesar_filas:

movups xmm0, [r9]  ; segunda fila
movups xmm1, [r10] ; anteúltima fila
cmp esi, 2
jne .end_if_0
  call change_sign_of_single_packed_xmm0_and_xmm1
.end_if_0:
movups [r8], xmm0  ; primera fila
movups [r11], xmm1 ; última fila

add r8, window_size
add r9, window_size
add r10, window_size
add r11, window_size

ret

procesar_columnas:

; leer columna izquierda

lea r15, [r12 - cell_size]
pinsrd xmm0, [r12], 0
add r12, r14
pinsrd xmm0, [r12], 1
add r12, r14
pinsrd xmm0, [r12], 2
add r12, r14
pinsrd xmm0, [r12], 3
add r12, r14

; leer columna derecha

lea rbx, [r13 + cell_size]
pinsrd xmm1, [r13], 0
add r13, r14
pinsrd xmm1, [r13], 1
add r13, r14
pinsrd xmm1, [r13], 2
add r13, r14
pinsrd xmm1, [r13], 3
add r13, r14

; cambiar el signo a ambas columnas si corresponde

cmp esi, 1
jne .end_if_1
  call change_sign_of_single_packed_xmm0_and_xmm1
.end_if_1:

; guardar columna izquierda

movss [r15], xmm0

add r15, r14
psrldq xmm0, 4
movss [r15], xmm0

add r15, r14
psrldq xmm0, 4
movss [r15], xmm0

add r15, r14
psrldq xmm0, 4
movss [r15], xmm0

; guardar columna derecha

movss [rbx], xmm1

add rbx, r13
psrldq xmm1, 4
movss [rbx], xmm1

add rbx, r13
psrldq xmm1, 4
movss [rbx], xmm1

add rbx, r13
psrldq xmm1, 4
movss [rbx], xmm1

ret

procesar_esquinas:

movss xmm0, [rbx + cell_size] ; 2da celda, primera fila
pinsrd xmm0, [r8 - cell_size], 1 ; anteúltima celda, primera fila
pinsrd xmm0, [r12], 2 ; 2da celda, última fila
pinsrd xmm0, [r13], 3 ; anteúltima celda, última fila

movss xmm1, [rbx + r14] ; 1ra celda, segunda fila
pinsrd xmm1, [r9], 1 ; última celda, segunda fila
neg r14
pinsrd xmm1, [r10 + r14 + cell_size], 2 ; 1ra celda, anteúltima fila
pinsrd xmm1, [r10], 3 ; última celda, anteúltima fila

addps xmm0, xmm1
divps xmm0, [cuatro_floats_contiguos_de_valor_dos]

movss [rdi], xmm0 ; esquina superior izquierda

psrldq xmm0, 4
movss [r8], xmm0 ; esquina superior derecha

psrldq xmm0, 4
movss [r12 - cell_size], xmm0 ; esquina inferior izquierda

psrldq xmm0, 4
movss [r11], xmm0 ; esquina inferior derecha 

ret

;global solver_project
;solver_project:
; void solver_project ( fluid_solver* solver, float * p, float * div )
; rdi = fluid_solver* solver
; rsi = float* p
; rdx = float* div
;ret