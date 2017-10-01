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

mov r13, rdi ;                                r13 -> solver
mov r14, rsi ;                                r14 -> p 
mov r15, rdx ;                                r15 -> div
mov ebx, [r13 + offset_N] ;                   rbx = N

; Configuración inicial para el ciclo 1

mov r11d, ebx
sar r11d, 2 ;                                 r11 = N/4

mov rax, rbx
mul eax
mov ecx, eax ;                                rcx = N^2

mov r10, rbx ;                                r10 = N
inc r10 ;                                     r10 = N+1
inc r10 ;                                     r10 = N+2
sal r10, 2 ;                                  r10 = (N+2)*4

mov rax, [r13 + offset_v]
add rax, float_size ;                         rax -> v(1, 0)
lea rdx, [rax + 2*r10] ;                      rdx -> v(1, 2)
mov rdi, [r13 + offset_u]
add rdi, r10 ;                                rdi -> u(0, 1)
lea rsi, [rdi + 2*float_size] ;               rsi -> u(2, 1)
lea r8, [r15 + r10 + float_size] ;            r8 -> div(1, 1)
lea r9, [r14 + r10 + float_size] ;            r9 -> p(1, 1)

xor r10, r10 ;                                r10 = 0

pxor xmm0, xmm0 ; para blanquear la matriz p

cvtsi2ss xmm1, ebx ; convierte uint32_t a float
movss xmm2, xmm1
pslldq xmm2, float_size
movss xmm2, xmm1
pslldq xmm2, float_size
movss xmm2, xmm1
pslldq xmm2, float_size
movss xmm2, xmm1 ;                                      xmm2 = N | N | N | N
movups xmm1, [constante_unmedio_negativo_para_solver] ; xmm1 = -1/2 | -1/2 | -1/2 | -1/2
divps xmm1, xmm2 ;                                      xmm1 = -1/2N | -1/2N | -1/2N | -1/2N
; el valor de xmm2 se puede descartar, xmm1 se va a usar en el ciclo1

; Valores antes del ciclo ("->" significa "puntero a")
; rcx = N^2                            ; para el loop
; rax -> v(1, 0)                       ; puntero a matriz para ciclo1
; rdx -> v(1, 2)                       ; puntero a matriz para ciclo1
; rdi -> u(0, 1)                       ; puntero a matriz para ciclo1
; rsi -> u(2, 1)                       ; puntero a matriz para ciclo1
; r8 -> div(1, 1)                      ; puntero a matriz para ciclo1
; r9 -> p(1, 1)                        ; puntero a matriz para ciclo1
; r10 = 0                              ; contador de 0 a N/4 para saltar en los bordes
; r11 = N/4                            ; para comparar con r10
; rbx = N                              ; no hace falta preservarlo pero se usa en la configuración inicial
; r12                                  ; este registro no fue utilizado
; r13 -> solver                        ; para preservarlo para llamar a solver_set_bnd
; r14 -> p                             ; para preservarlo para llamar a solver_set_bnd
; r15 -> div                           ; para preservarlo para llamar a solver_set_bnd
; xmm0 = 0 | 0 | 0 | 0                 ; para blanquear la matriz p
; xmm1 = -1/2N | -1/2N | -1/2N | -1/2N ; para actualizar la matriz div
.ciclo1:
  ; Blanqueo de matriz p:
  movdqu [r9], xmm0

  ; Actualización de matriz div:
  movups xmm2, [rdx]
  movups xmm3, [rax]
  subps xmm2, xmm3
  movups xmm3, [rdi]
  addps xmm2, xmm3
  movups xmm3, [rsi]
  subps xmm2, xmm3
  mulps xmm2, xmm1
  movups [r8], xmm2

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
  jne .end_if_0
    xor r10, r10
    add rax, 2*float_size
    add rdx, 2*float_size
    add rdi, 2*float_size
    add rsi, 2*float_size
    add r8, 2*float_size
    add r9, 2*float_size
  .end_if_0:
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

mov rdi, r13 ; rdi -> solver
xor rsi, rsi ; rsi = 0
mov rdx, r14 ; rdx -> p
mov rcx, r15 ; rcx -> div
xor r8, r8
inc r8       ; r8 = 1
mov r9, 4    ; r9 = 4
call solver_lin_solve

; Configuración inicial para el ciclo 2

mov r11d, ebx
sar r11d, 2 ;                                 r11 = N/4

mov rax, rbx
mul eax
mov ecx, eax ;                                rcx = N^2

mov r10, rbx ;                                r10 = N
inc r10 ;                                     r10 = N+1
inc r10 ;                                     r10 = N+2
sal r10, 2 ;                                  r10 = (N+2)*4

mov rax, [r13 + offset_v]
lea rax, [rax + r10 + 2*float_size] ;         rax -> v(1, 1)
mov rdx, [r13 + offset_u]
lea rdx, [rdx + r10 + 2*float_size] ;         rdx -> u(1, 1)
lea rdi, [r14 + float_size] ;                 rdi -> p(1, 0)
lea rsi, [r14 + 2*r10 + float_size] ;         rsi -> p(1, 2)
lea r8, [r14 + float_size] ;                  r8 -> p(0, 1)
lea r9, [r14 + 2*r10 + float_size] ;          r9 -> p(2, 1)

xor r10, r10 ;                                r10 = 0

cvtsi2ss xmm0, ebx ; convierte uint32_t a float
movss xmm1, xmm0
pslldq xmm1, float_size
movss xmm1, xmm0
pslldq xmm1, float_size
movss xmm1, xmm0
pslldq xmm1, float_size
movss xmm1, xmm0 ;                                      xmm1 = N | N | N | N
movups xmm0, [constante_unmedio_negativo_para_solver] ; xmm0 = -1/2 | -1/2 | -1/2 | -1/2
mulps xmm0, xmm1 ;                                      xmm0 = -N/2 | -N/2 | -N/2 | -N/2
; el valor de xmm1 se puede descartar, xmm0 se va a usar en el ciclo2

; Valores antes del ciclo ("->" significa "puntero a")
; rcx = N^2                            ; para el loop
; rax -> v(1, 1)                       ; puntero a matriz para ciclo1
; rdx -> u(1, 1)                       ; puntero a matriz para ciclo1
; rdi -> p(1, 0)                       ; puntero a matriz para ciclo1
; rsi -> p(1, 2)                       ; puntero a matriz para ciclo1
; r8 -> p(0, 1)                        ; puntero a matriz para ciclo1
; r9 -> p(2, 1)                        ; puntero a matriz para ciclo1
; r10 = 0                              ; contador de 0 a N/4 para saltar en los bordes
; r11 = N/4                            ; para comparar con r10
; rbx = N                              ; no hace falta preservarlo pero se usa en la configuración inicial
; r12                                  ; este registro no fue utilizado
; r13 -> solver                        ; para preservarlo para llamar a solver_set_bnd
; r14 -> p                             ; para preservarlo para llamar a solver_set_bnd
; r15 -> div                           ; para preservarlo para llamar a solver_set_bnd
; xmm0 = 0 | 0 | 0 | 0                 ; para blanquear la matriz p
; xmm1 = -1/2N | -1/2N | -1/2N | -1/2N ; para actualizar la matriz div

.ciclo2:
  ; Actualización de matriz u:
  movups xmm1, [r9]
  movups xmm2, [r8]
  subps xmm1, xmm2
  mulps xmm1, xmm0
  movups xmm2, [rdx]
  addps xmm1, xmm2
  movups [rdx], xmm1

  ; Actualización de matriz v:
  movups xmm1, [rsi]
  movups xmm2, [rdi]
  subps xmm1, xmm2
  mulps xmm1, xmm0
  movups xmm2, [rax]
  addps xmm1, xmm2
  movups [rax], xmm1

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
  jne .end_if_1
    xor r10, r10
    add rax, 2*float_size
    add rdx, 2*float_size
    add rdi, 2*float_size
    add rsi, 2*float_size
    add r8, 2*float_size
    add r9, 2*float_size
  .end_if_1:
loop .ciclo2

mov rdi, r13 ; rdi -> solver
xor rsi, rsi
inc rsi      ; rsi = 1
mov rdx, [r13 + offset_u] ; rdx -> div
call solver_set_bnd

mov rdi, r13 ; rdi -> solver
mov rsi, 2   ; rsi = 2
mov rdx, [r13 + offset_v] ; rdx -> p
call solver_set_bnd

pop r15
pop r14
pop r13
pop r12
pop rbx

ret