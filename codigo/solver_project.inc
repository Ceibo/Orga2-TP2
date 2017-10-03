global solver_project
solver_project:
;void solver_project ( fluid_solver* solver, float * p, float * div )
;rdi = fluid_solver* solver
;rsi = float* p
;rdx = float* div

push rbx ; pila alineada
push r12
push r13 ; pila alineada
push r14
push r15 ; pila alineada

; Datos que se preservan en los call

mov r13, rdi ;              r13 -> solver
mov r14, rsi ;              r14 -> p 
mov r15, rdx ;              r15 -> div
mov ebx, [r13 + offset_N] ; rbx = N

; Configuración inicial para el ciclo 1

mov r11d, ebx
sar r11d, 2 ;  r11 = N/4 ; para el loop, para saltar los bordes

mov rax, rbx ; rax = N
mul r11d ;     rax = N*(N/4) = (N^2)/4
mov ecx, eax ; rcx = (N^2)/4 ; para el loop, proceso de a 4 elementos

mov r10, rbx ; r10 = N
inc r10 ;      r10 = N+1
inc r10 ;      r10 = N+2
sal r10, 2 ;   r10 = (N+2)*4 ; tiene el tamaño de una fila, para hacer saltos

mov rax, [r13 + offset_v]
add rax, float_size ;              rax -> v(1, 0)
lea rdx, [rax + 2*r10] ;           rdx -> v(1, 2)
mov rdi, [r13 + offset_u]
add rdi, r10 ;                     rdi -> u(0, 1)
lea rsi, [rdi + 2*float_size] ;    rsi -> u(2, 1)
lea r8, [r15 + r10 + float_size] ; r8 -> div(1, 1)
lea r9, [r14 + r10 + float_size] ; r9 -> p(1, 1)

xor r10, r10 ;                     r10 = 0

pxor xmm0, xmm0 ; para blanquear la matriz p

cvtsi2sd xmm1, ebx ; convierte uint32_t a float
movsd xmm2, xmm1
pslldq xmm2, double_float_size
movsd xmm2, xmm1 ;                                               xmm2 = N | N
movupd xmm1, [dquad_con_dos_double_de_valor_un_medio_negativo] ; xmm1 = -1/2 | -1/2

; Valores antes del ciclo ("->" significa "puntero a")
; rcx = (N^2)/4        ; para el loop
; rax -> v(1, 0)       ; puntero a matriz para ciclo1
; rdx -> v(1, 2)       ; puntero a matriz para ciclo1
; rdi -> u(0, 1)       ; puntero a matriz para ciclo1
; rsi -> u(2, 1)       ; puntero a matriz para ciclo1
; r8 -> div(1, 1)      ; puntero a matriz para ciclo1
; r9 -> p(1, 1)        ; puntero a matriz para ciclo1
; r10 = 0              ; contador de 0 a N/4 para saltar en los bordes
; r11 = N/4            ; para comparar con r10
; rbx = N              ; no hace falta preservarlo pero se usa en la configuración inicial
; r12                  ; este registro no fue utilizado
; r13 -> solver        ; para preservarlo para llamar a solver_set_bnd
; r14 -> p             ; para preservarlo para llamar a solver_set_bnd
; r15 -> div           ; para preservarlo para llamar a solver_set_bnd
; xmm0 = 0 | 0 | 0 | 0 ; para blanquear la matriz p
; xmm1 = -1/2 | -1/2   ; para actualizar la matriz div
; xmm2 = N | N         ; para actualizar la matriz div

.ciclo1:
  call proceso_ciclo1
loop .ciclo1

; Llamados a funciones

mov rdi, r13 ; rdi -> solver
xor rsi, rsi ; rsi = 0
mov rdx, r15 ; rdx -> div
call solver_set_bnd

mov rdi, r13 ; rdi -> solver
xor rsi, rsi ; rsi = 0
mov rdx, r14 ; rdx -> p
call solver_set_bnd

mov rdi, r13                           ; rdi -> solver
xor rsi, rsi                           ; rsi = 0
mov rdx, r14                           ; rdx -> p
mov rcx, r15                           ; rcx -> div
movss xmm0, [dword_float_valor_uno]    ; xmm0 = 1.0f
movss xmm1, [dword_float_valor_cuatro] ; xmm1 = 4.0f
call solver_lin_solve

; Configuración inicial para el ciclo 2

mov r11d, ebx
sar r11d, 2 ;  r11 = N/4 ; para el loop, para saltar los bordes

mov rax, rbx ; rax = N
mul r11d ;     rax = N*(N/4) = (N^2)/4
mov ecx, eax ; rcx = (N^2)/4 ; proceso de a 4 elementos

mov r10, rbx ; r10 = N
inc r10 ;      r10 = N+1
inc r10 ;      r10 = N+2
sal r10, 2 ;   r10 = (N+2)*4 ; tiene el tamaño de una fila, para hacer saltos

mov rax, [r13 + offset_v]
lea rax, [rax + r10 + 2*float_size] ; rax -> v(1, 1)
mov rdx, [r13 + offset_u]
lea rdx, [rdx + r10 + 2*float_size] ; rdx -> u(1, 1)
lea rdi, [r14 + float_size] ;         rdi -> p(1, 0)
lea rsi, [rdi + 2*r10] ;              rsi -> p(1, 2)
lea r8, [r14 + r10] ;                 r8 -> p(0, 1)
lea r9, [r8 + 2*float_size] ;         r9 -> p(2, 1)

xor r10, r10 ; r10 = 0 ; contador para comparar con r11

cvtsi2sd xmm0, ebx ; convierte uint32_t a float
movsd xmm1, xmm0
pslldq xmm1, double_float_size
movsd xmm1, xmm0 ;                                      xmm1 = N | N
movupd xmm0, [dquad_con_dos_double_de_valor_un_medio] ; xmm0 = 1/2 | 1/2
mulpd xmm0, xmm1 ;                                      xmm0 = N/2 | N/2
; el valor de xmm1 se puede descartar, xmm0 se va a usar en el ciclo2

; Valores antes del ciclo ("->" significa "puntero a")
; rcx = (N^2)/4    ; para el loop
; rax -> v(1, 1)   ; puntero a matriz para ciclo1
; rdx -> u(1, 1)   ; puntero a matriz para ciclo1
; rdi -> p(1, 0)   ; puntero a matriz para ciclo1
; rsi -> p(1, 2)   ; puntero a matriz para ciclo1
; r8 -> p(0, 1)    ; puntero a matriz para ciclo1
; r9 -> p(2, 1)    ; puntero a matriz para ciclo1
; r10 = 0          ; contador de 0 a N/4 para saltar en los bordes
; r11 = N/4        ; para comparar con r10
; rbx = N          ; no hace falta preservarlo pero se usa en la configuración inicial
; r12              ; este registro no fue utilizado
; r13 -> solver    ; para preservarlo para llamar a solver_set_bnd
; r14 -> p         ; para preservarlo para llamar a solver_set_bnd
; r15 -> div       ; para preservarlo para llamar a solver_set_bnd
; xmm0 = N/2 | N/2 ; para actualizar las matrices u y v

.ciclo2:
  call proceso_ciclo2
loop .ciclo2

mov rdi, r13 ; rdi -> solver
xor rsi, rsi
inc rsi      ; rsi = 1
mov rdx, [r13 + offset_u] ; rdx -> u
call solver_set_bnd

mov rdi, r13 ; rdi -> solver
mov rsi, 2   ; rsi = 2
mov rdx, [r13 + offset_v] ; rdx -> v
call solver_set_bnd

pop r15
pop r14
pop r13
pop r12
pop rbx

ret

proceso_ciclo1:
; Blanqueo de matriz p:
movdqu [r9], xmm0

; Actualización de matriz div:
movups xmm3, [rsi]  ; xmm3 = u(i+1, j) = A | B | C | D
cvtps2pd xmm4, xmm3 ; xmm4 = C | D (con doble precisión)
psrldq xmm3, double_float_size ; xmm3 = 0 | 0 | A | B
cvtps2pd xmm5, xmm3 ; xmm5 = A | B (con doble precisión)

movups xmm3, [rdi] ; xmm3 = u(i-1, j) = A' | B' | C' | D'
cvtps2pd xmm6, xmm3 ; xmm6 = C' | D' (con doble precisión)
psrldq xmm3, double_float_size ; xmm3 = 0 | 0 | A' | B'
cvtps2pd xmm7, xmm3 ; xmm7 = A' | B' (con doble precisión)

; xmm5:xmm4 = u(i+1, j) - u(i-1, j)
subpd xmm4, xmm6
subpd xmm5, xmm7

movups xmm3, [rdx] ; xmm3 = v(i, j+1)
cvtps2pd xmm6, xmm3
psrldq xmm3, double_float_size
cvtps2pd xmm7, xmm3

; xmm5:xmm4 = u(i+1, j) - u(i-1, j) + v(i, j+1)
addpd xmm4, xmm6
addpd xmm5, xmm7

movups xmm3, [rax] ; xmm3 = v(i, j-1)
cvtps2pd xmm6, xmm3
psrldq xmm3, double_float_size
cvtps2pd xmm7, xmm3

; xmm5:xmm4 = u(i+1, j) - u(i-1, j) + v(i, j+1) - v(i, j-1)
subpd xmm4, xmm6
subpd xmm5, xmm7

; xmm5:xmm4 = (-1/2) * [v(i, j+1) - v(i, j-1) + u(i+1, j) - u(i-1, j)]
mulpd xmm4, xmm1
mulpd xmm5, xmm1

; xmm5:xmm4 = (-1/2) * [v(i, j+1) - v(i, j-1) + u(i+1, j) - u(i-1, j)] / N
divpd xmm4, xmm2 ; xmm4 = A | B
divpd xmm5, xmm2 ; xmm5 = C | D

cvtpd2ps xmm3, xmm4            ; xmm3 = 0 | 0 | C | D
cvtpd2ps xmm4, xmm5            ; xmm4 = 0 | 0 | A | B
pslldq xmm4, double_float_size ; xmm4 = A | B | 0 | 0
por xmm3, xmm4                 ; xmm3 = A | B | C | D

movups [r8], xmm3

; Aumentamos 4 celdas cada puntero
add rax, xmm_size
add rdx, xmm_size
add rdi, xmm_size
add rsi, xmm_size
add r8, xmm_size
add r9, xmm_size

; Si estamos parados en la última celda de una fila, 
; hay que saltar 2 celdas más para saltar el borde
inc r10
cmp r10, r11 ; if r10 == N/4
jne .end_if
  xor r10, r10
  add rax, 2*float_size
  add rdx, 2*float_size
  add rdi, 2*float_size
  add rsi, 2*float_size
  add r8, 2*float_size
  add r9, 2*float_size
.end_if:
ret

proceso_ciclo2:
; ACTUALIZACIÓN DE MATRIZ U:
movups xmm1, [r9]   ; xmm1 = p(i+1, j) = A | B | C | D
cvtps2pd xmm2, xmm1 ; xmm2 = C | D (con doble precisión)
psrldq xmm1, double_float_size ; xmm1 = 0 | 0 | A | B
cvtps2pd xmm3, xmm1 ; xmm3 = A | B (con doble precisión)

movups xmm1, [r8]   ; xmm1 = p(i-1, j) = A' | B' | C' | D'
cvtps2pd xmm4, xmm1 ; xmm4 = C' | D' (con doble precisión)
psrldq xmm1, double_float_size ; xmm1 = 0 | 0 | A' | B'
cvtps2pd xmm5, xmm1 ; xmm5 = A' | B' (con doble precisión)

; xmm3:xmm2 = p(i+1, j) - p(i-1, j)
subpd xmm2, xmm4
subpd xmm3, xmm5

; xmm3:xmm2 = N/2*[p(i+1, j) - p(i-1, j)]
mulpd xmm2, xmm0 ; C | D
mulpd xmm3, xmm0 ; A | B

movups xmm1, [rdx]  ; xmm1 = u(i, j)
cvtps2pd xmm4, xmm1 ; C' | D'
psrldq xmm1, double_float_size
cvtps2pd xmm5, xmm1 ; A' | B'

; xmm5:xmm4 = u(i, j) - N/2*[p(i+1, j) - p(i-1, j)]
subpd xmm4, xmm2 ; C | D
subpd xmm5, xmm3 ; A | B

cvtpd2ps xmm2, xmm4 ; xmm2 = 0 | 0 | C | D
cvtpd2ps xmm3, xmm5 ; xmm3 = 0 | 0 | A | B
pslldq xmm3, double_float_size ; xmm3 = A | B | 0 | 0
por xmm2, xmm3

movups [rdx], xmm2 ; u(i, j) -= N/2*[p(i+1, j) - p(i-1, j)]

; ACTUALIZACIÓN DE MATRIZ V:
movups xmm1, [rsi]  ; xmm1 = p(i, j+1) = A | B | C | D
cvtps2pd xmm2, xmm1 ; xmm2 = C | D (con doble precisión) 
psrldq xmm1, double_float_size ; xmm1 = 0 | 0 | A | B
cvtps2pd xmm3, xmm1 ; xmm1 = A | B

movups xmm1, [rdi]  ; xmm1 = p(i, j-1) = A' | B' | C' | D'
cvtps2pd xmm4, xmm1 ; xmm4 = C' | D' (con doble precisión) 
psrldq xmm1, double_float_size ; xmm1 = 0 | 0 | A' | B'
cvtps2pd xmm5, xmm1 ; xmm5 = A' | B'

; xmm3:xmm2 = p(i, j+1) - p(i, j-1)
subpd xmm2, xmm4 ; C | D
subpd xmm3, xmm5 ; A | B

; xmm3:xmm2 = N/2*[p(i, j+1) - p(i, j-1)]
mulpd xmm2, xmm0 ; C | D
mulpd xmm3, xmm0 ; A | B

movups xmm1, [rax]  ; xmm2 = v(i, j)
cvtps2pd xmm4, xmm1 ; C' | D' 
psrldq xmm1, double_float_size
cvtps2pd xmm5, xmm1 ; A' | B'

; xmm5:xmm4 = v(i, j) - N/2*[p(i, j+1) - p(i, j-1)]
subpd xmm4, xmm2
subpd xmm5, xmm3

cvtpd2ps xmm2, xmm4 ; xmm2 = 0 | 0 | C | D
cvtpd2ps xmm3, xmm5 ; xmm3 = 0 | 0 | A | B
pslldq xmm3, double_float_size ; xmm3 = A | B | 0 | 0
por xmm2, xmm3

movups [rax], xmm2 ; v(i, j) -= N/2*[p(i, j+1) - p(i, j-1)]

; Aumentamos 4 celdas cada puntero
add rax, xmm_size
add rdx, xmm_size
add rdi, xmm_size
add rsi, xmm_size
add r8, xmm_size
add r9, xmm_size

; Si estamos parados en la última celda de una fila, 
; hay que saltar 2 celdas más para saltar el borde
inc r10
cmp r10, r11 ; if r10 == N/4
jne .end_if
  xor r10, r10
  add rax, 2*float_size
  add rdx, 2*float_size
  add rdi, 2*float_size
  add rsi, 2*float_size
  add r8, 2*float_size
  add r9, 2*float_size
.end_if:
ret