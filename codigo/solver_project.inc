global solver_project
solver_project:
;void solver_project ( fluid_solver* solver, float * p, float * div )
;rdi = fluid_solver* solver
;rsi = float* p
;rdx = float* div

push rbx
push r12
push r13
push r14
push r15

mov r13, rdi ;                                r13 -> solver
mov r14, rsi ;                                r14 -> p 
mov r15, rdx ;                                r15 -> div
mov ebx, [r13 + offset_N] ;                   rbx = N

mov r11d, ebx
sar r11d, 2 ;                                 r11 = N/4

mov rax, rbx
mul eax
mov ecx, eax ;                                rcx = N^2

mov rax, [r13 + offset_v]
add rax, float_size ;                         rax -> v(1, 0)

mov r10, rbx ;                                r10 = N
inc r10 ;                                     r10 = N+1
inc r10 ;                                     r10 = N+2
sal r10, 2 ;                                  r10 = (N+2)*4

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
; rbx = N                              ; no se usa en ciclo1
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
  subps xmm2, [rax]
  addps xmm2, [rdi]
  subps xmm2, [rsi]
  mulps xmm2, xmm1
  movups [r8], xmm2

  ; Si estamos parados en la anteúltima celda de una fila, 
  ; hay que saltar 3 celdas en lugar de cuatro, por eso hace sub
  inc r10
  cmp r10, r11 ; if r10 == N/4
  jne .end_if
    xor r10, r10
    sub rax, float_size
    sub rdx, float_size
    sub rdi, float_size
    sub rsi, float_size
    sub r8, float_size
    sub r9, float_size
  .end_if:

  ; Aumentamos 4 celdas cada puntero
  sub rax, xmm_size
  sub rdx, xmm_size
  sub rdi, xmm_size
  sub rsi, xmm_size
  sub r8, xmm_size
  sub r9, xmm_size
loop .ciclo1

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



; FOR_EACH_CELL
; solver->u[IX(i,j)] -= 0.5f*solver->N*(p[IX(i+1,j)]-p[IX(i-1,j)]);
; solver->v[IX(i,j)] -= 0.5f*solver->N*(p[IX(i,j+1)]-p[IX(i,j-1)]);
; END_FOR
;x -= 5 es igual a x = x - 5;
mov rdi, r12; Vuelvo a restaurar rdi
mov rax, [rdi + offset_u]; Pongo en eax puntero al primer elemento de la matriz u
mov r10, [rdi + offset_v]; Pongo en r10d el puntero al primer elemento de la matriz v
xor rsi, rsi;Pongo rsi en 0, ahora le voy a dar otro uso
mov esi, [rdi + offset_N]; Pongo en esi (32bits) el N, tamaño de la matriz sin contar los bordes(uint32) 
add esi, 2; Ahora tengo N+2. El tamaño posta
add rax, rsi; Avanzo una fila en u. No uso el esi por que son de dist tamaño, pero está todo bien por que 
;estaba en 0 la parte alta de rsi.
add rax, float_size; Avanzo un casillero de la matriz. Ahora estoy en (1,1)
add r10, rsi
add r10, float_size; Hago lo mismo con v. En r10 tengo puntero a (1,1) de matriz v
mov r8, r13; Guardo en r8 el puntero a P, la matriz que voy a recorrer ahora
add r8, rsi; Avanzo fila en matriz P 
add r8, float_size; Me pongo en (1,1) en la matriz P
mov r13, r8; Tengo en r13 el (1,1) de P. Lo voy a usar después y no lo voy a necesitar mas
mov ecx, ebx; Pongo en ecx (N+2)^2, para el LOOP
;Ahora ya tengo todo como para entrar al ciclo

.ciclo2: 
;FOR_EACH_CELL
;solver->u[IX(i,j)] -= 0.5f*solver->N*(p[IX(i+1,j)]-p[IX(i-1,j)]);
add r8, float_size; Avanzo uno en r8 (i+1,j)
sub r8, rsi
movups xmm1, [r8]; Pongo en xmm1 (i-1,j) y los 3 siguientes
add r8, rsi
add r8, rsi
movups xmm2, [r8]
subps xmm2, xmm1;(i+1,j)-(i-1,j)
mov r9d, esi; Pongo en r9d N+2
sub r9d, 2; Tengo N en esi
movd xmm3, r9d
pslld xmm3, float_size
movd xmm3, r9d
pslld xmm3, float_size
movd xmm3, r9d
pslld xmm3, float_size; Ahora tengo en xmm3 algo así:
movd xmm3, r9d; 		[N, N, N, N]
;Ahora multiplico por N
mulps xmm2, xmm3
mulps xmm2, [constante_unmedio_negativo_para_solver]
;Pero eso era 1/2 negativo, y necesito positivo, pero lo que hacía era un -= 
;o sea que ahora tendría simplemente hacer un add, por que lo negativo ya está.
;En eax tenía puntero a u en (1,1)
;Y en xmm2 tengo el resultado buscado
movups [rax], xmm2; LISTO!(esta parte)

;Ahora vamos con esto
;solver->v[IX(i,j)] -= 0.5f*solver->N*(p[IX(i,j+1)]-p[IX(i,j-1)]);
;v->r10, p *en (1,1)->r13. AHORA ME MUEVO POR "FILAS"
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