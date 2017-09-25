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

section .data

dquad_con_bits_de_signo_float_en_1_y_el_resto_en_0: DD 0x80000000, 0x80000000, 0x80000000, 0x80000000

section .text

%include "solver_set_bnd_functions.inc"

;*************************************************************************************************


global solver_lin_solve

%define offset_a -4
%define offset_c -8

solver_lin_solve:
; void solver_lin_solve ( fluid_solver* solver, uint32_t b, float * x, float * x0, float a, float c )
; rdi = fluid_solver* solver
; esi = uint32_t b
; rdx = float* x
; rcx = float* x0
; xmm0 = float a ; primeros 4 bytes
; xmm1 = float c ; primeros 4 bytes
	push rbp; alineada
	mov rbp,rsp;
	sub rsp,8; 
	push rbx;alineada
	push r12; 
	push r13; alineada
	push r14; 
	push r15;  alineada
	
	
	cmp rdi,0
	je .fin
	cmp rdx ,0
	je .fin
	cmp rcx,0
	je .fin; 
	
;backups de argumentos iniciales
	mov rbx, rdi ;                                           ** rbx <------------ solver **
	mov r12,rcx;                                              ** r12 <--------------- x0 **
	mov r13, rdx;                                               ** r13 <----------------- x **
	xor r14,r14
	mov r14d,esi ;                                              ** r14d <----------------- b **
    movss [rbp+offset_a],xmm0 ;                      ** rbp+offset_a <------- backup de a **
    movss [rbp+offset_c],xmm1 ;                      ** rbp+offset_c <------- backup de c **
    xor r15,r15
    mov r15,0
    
.ciclo_k: ; ciclo externo que itera sobre k desde 1 hasta 20
	cmp r15,20
	je .fin
	
	mov r8,r13; r8 puntero a x 
	mov r9, r12 ; r9 puntero a x0
	xor rdx,rdx
	xor rax,rax
	xor r10,r10
	mov eax,[rdi+offset_N] ; eax = N
	mov r10d,eax;                               ** r10d <--------  N **
	inc eax; eax = N+1
	xor rcx,rcx
	mov ecx,4
	mul ecx; edx : eax = (N+1)*4
	shl rax,32; ** agregado **
    shrd rax,rdx,32;** agregado ** rax = edx:eax
    mov rcx,rax;                                 ** rcx <------ (N+1)*4 **
	xor r11,r11
	mov r11d,4
	xor rax,rax
	mov rax,r10; eax = N
	div r11d, ; dividimos N por 4,                ***  eax <------ cociente , edx <----- resto *****
    xor rsi,rsi
    mov esi,eax; esi backup de cociente
    inc eax    ;                                 ** eax = (N/4) + 1 **
 	inc r10d     ;                              **   r10d = N+1  **
	add r8,4;                                                 ** r8 apunta a x(1,0) **
	add r9,rcx ;r9 apunta a (N+1) Esima posiciOn
	add r9,8;  r9 apunta a (N+3) Esima posiciOn                 ** r9 apunta a x0(1,1) **
	movss xmm0,[rbp+offset_a] ;                 **   xmm0 <----------- a  **
	movss xmm1,[rbp+offset_c] ;                  **  xmm1 <---------- c **

;guarda de condicional: 
	cmp esi,0
	je .fin; matriz vacIa
	xor r11,r11 ;
	mov r11d,1  ;   i en 1                         r11d <---------- i 
	xor rsi,rsi ; 
	mov esi,1 ;     j en 1                         esi <---------- j 
	
.ciclo_principal: 
    
;guarda del ciclo_principal:
	cmp r11d,eax
	je .seguir; si i es N/4 +1 entonces finalizamos ciclo
		 
;cuerpo del ciclo_principal: 

.subciclo_4_en_paralelo: 

;guarda:
	cmp esi,r10d; r10d = N + 1
	je .fin_de_ciclo_principal; si j es N+1 entonces saltar a fin_de_ciclo_principal
	
;sobre la matriz x:
	cmp esi,1
	jne .usar_backup; si no es primer fila entonces usamos registro backup para piso
	movups xmm3 , [r8]; xmm3 fetch piso => xmm3 = x(i+3,j-1)| x(i+2,j-1)| x(i+1,j-1) |x(i,j-1)
    jmp .continuar_ciclo

.usar_backup:
 	movups xmm3 , xmm2;  ******* xmm2 es backup de resultado *********
 	
.continuar_ciclo:
	add r8,rcx; rcx <------ (N+1)*4 
 	movups xmm4, [r8]; xmm4 fetch medio y lado left => xmm4 = x(i+2,j)|x(i+1,j)|x(i,j)|x(i-1,j)
    movups xmm5, [r8+8]; xmm5 fetch lado right => xmm5 = x(i+4,j)|x(i+3,j)|x(i+2,j)|x(i+1,j)
    add r8,rcx ; r8 + (N+1)
    add r8,8; r8 apunta a N+3 Esima posiciOn => r8 = x(i,j+1)
    movups xmm6, [r8]; xmm6 fetch techo => xmm6 = x(i+3,j+1)|x(i+2,j+1)|x(i+1,j+1)|x(i,j+1)
    sub r8,rcx; 
    sub r8, rcx; r8 - 2*(N+1)*4 => r8 - (2*N + 2)*4
    sub r8, 8;    r8 - (2*N + 4)*4  , restauramos puntero a i inicial
    
;sobre la matriz x0:
    movups xmm7,[r9];r9 apunta a x0(i,j) => xmm7 = x0(i+3,j)|x0(i+2,j)|x0(i+1,j)|x0(i,j)
    
;procedimiento:
     ; xmm6 = techo, xmm5 = right, xmm3 = piso 
 
;convertimos single fp a double fp:
	 cvtps2pd xmm8,xmm6 ; xmm8 = x(i+1,j+1) |  x(i,j+1) ,1ro y 2d0 single xmm6 a 2 double en xmm8
	 psrldq xmm6,8 ; xmm6 = . | . |x(i+3,j+1)|x(i+2,j+1) 
	 cvtps2pd xmm9,xmm6 ; xmm9 =x(i+3,j+1) | x(i+2,j+1)  ,3er y 4to single xmm6 a 2 double en xmm9
	 cvtps2pd xmm10,xmm5 ; xmm10 = x(i+2,j) | x(i+1,j) ,1ro y 2d0 single xmm5 a 2 double en xmm10
	 psrldq xmm5,8 ; xmm6 = . | . |x(i+4,j)|x(i+3,j)
	 cvtps2pd xmm11,xmm5 ; xmm11 = x(i+4,j) | x(i+3,j)  ,3er y 4to single xmm5 a 2 double en xmm11
	 cvtps2pd xmm12,xmm3 ; xmm12 = x(i+1,j-1) |x(i,j-1) ,1ro y 2d0 single xmm3 a 2 double en xmm12
	 psrldq xmm3,8 ; xmm6 = . | . |x(i+3,j-1)| x(i+2,j-1)
	 cvtps2pd xmm13,xmm3 ; xmm13 =x(i+3,j-1)| x(i+2,j-1) ,3er y 4to single xmm3 a 2 double en xmm13
	 addpd xmm8,xmm10; xmm8 = x(i+1,j+1)+x(i+2,j)|x(i,j+1)+x(i+1,j)
	 addpd xmm9,xmm11; xmm9 = x(i+3,j+1)+x(i+4,j)|x(i+2,j+1)+x(i+3,j)
	 addpd xmm8,xmm12 ; xmm8 = x(i+1,j+1)+x(i+2,j)+x(i+1,j-1)|x(i,j+1)+x(i+1,j)+x(i,j-1)
	 addpd xmm9,xmm13 ; xmm9 = x(i+3,j+1)+x(i+4,j)+x(i+3,j-1)|x(i+2,j+1)+x(i+3,j)+x(i+2,j-1)
	 ; xmm8 es parte baja (2 double) de suma parcial y xmm9 es parte alta (2 double) de suma parcial
	 cvtps2pd xmm3,xmm4; xmm3 = . |x(i-1,j) , 1er single xmm4 a double en xmm3
	 ;xmm3 4to operando double para cAlculo 1er punto
	 cvtps2pd xmm4,xmm7; xmm4 = x0(i+1,j)|x0(i,j)   ,1ro y 2d0 single xmm7 a 2 double en xmm4
	 ;xmm4 contiene 2do y 1er  double de fetch x0
	 psrldq xmm7,8 ; xmm7 = . | . |x0(i+3,j)|x0(i+2,j)
	 cvtps2pd xmm5,xmm7 ; xmm5 = x0(i+3,j)|x0(i+2,j) ,3er y 4to single xmm7 a 2 double en xmm5
	 ;xmm5 contiene 4to y 3er double de fetch x0
	 cvtss2sd xmm7,xmm0
	 ;xmm7 = .|a  , xmm7 es double a
	 cvtss2sd xmm10,xmm1
	 ;xmm10 = .|c  , xmm10 es double c
	 
;seteamos contador para loop
	 xor rdi,rdi
	 mov rdi,1
	 
.operacion:; (se muestra operaciOn sobre 1er punto)
	 cmp rdi,5
	 je .fin_operacion
	 addsd xmm3,xmm8 ; xmm3 = . | x(i,j+1)+x(i+1,j)+x(i,j-1)+x(i-1,j)  
	 mulsd xmm3,xmm7; xmm3 = . | ( x(i,j+1)+x(i+1,j)+x(i,j-1)+x(i-1,j) )*a
	 addsd xmm3,xmm4; xmm3 = . | ( ( x(i,j+1)+x(i+1,j)+x(i,j-1)+x(i-1,j) )*a ) + x0(i,j)  
	 divsd xmm3,xmm10; xmm3 = . | (( ( x(i,j+1)+x(i+1,j)+x(i,j-1)+x(i-1,j) )*a ) + x0(i,j) )/c
	 ;xmm3 es operando de siguiente punto
	 cvtsd2ss xmm11,xmm3 ; xmm11 = .|.|.|(( ( x(i,j+1)+x(i+1,j)+x(i,j-1)+x(i-1,j) )*a ) + x0(i,j) )/c
	 movss xmm2,xmm11; xmm2 = .|.|.|(( ( x(i,j+1)+x(i+1,j)+x(i,j-1)+x(i-1,j) )*a ) + x0(i,j) )/c
	 cmp rdi,4
	 je .fin_operacion; xmm2 contiene los 4 resultados en posiciones invertidas 
	 pslldq xmm2,4; xmm2 = .|.|(( ( x(i,j+1)+x(i+1,j)+x(i,j-1)+x(i-1,j) )*a ) + x0(i,j) )/c |.
	 
;preparamos registros para siguiente punto:
	 cmp rdi,2
	 je .actualizar_registros; si obtuvimos 2 resultados debemos actualizar registros
	 psrldq xmm8,4; xmm8 = .| x(i+1,j+1)+x(i+2,j)+x(i+1,j-1)
	 psrldq xmm4,4; xmm4 = .| x0(i+1,j)
	 inc rdi
	 jmp .operacion
	 
.actualizar_registros:
	 movupd xmm8,xmm9; xmm8 es parte alta (2 double) de suma parcial
	 movupd xmm4,xmm5; xmm4 contiene 4to y 3er double de fetch x0
	 inc rdi
	 jmp .operacion
	 
.fin_operacion:
	 movups xmm12,xmm2 ; xmm12 = punto_1 | punto_2 | punto_3 | punto_4
	 pshufd xmm2,xmm12,00011011b ; xmm2 = punto_4 | punto_3 | punto_2 | punto_1 , invertimos xmm2
	 movups [r8],xmm2; updateamos 4 puntos consecutivos de x
	 add r8,rcx; r8 apunta a N+1 Esima posiciOn 
	 add r8,4 ; r8 apunta a N+2 Esima posiciOn ; actualizado x
	 add r9,rcx; r9 apunta a N+1 Esima posiciOn
	 add r9,4 ; r9 apunta a N+2 Esima posiciOn ; actualizado x0
	 inc esi; incrementamos j
	 jmp .subciclo_4_en_paralelo
	 
.fin_de_ciclo_principal:
	 inc r11d; incrementamos i
	 add r8,4;   r8 apunta a segundo elemento de x 
	 add r9,rcx ;r9 apunta a (N+1) Esima posiciOn
	 add r9,8;  r9 apunta a (N+3) Esima posiciOn    ,r9 apunta a (N+3) elemento de x0
	 xor rdx,r11; rdx = i
	 sal rdx,2 ; rdx = i*4
	 add r8,rdx; r8 incrementado en 4 posiciones
	 add r9,rdx; r9 incrementado en 4 posiciones
	 jmp .ciclo_principal
	 
.seguir:

;llamar funciOn solver_set_bnd:
	xor rdi
	xor rsi
	xor rdx
	mov rdi,rbx;rbx <------------ solver , primer parAmetro solver_set_bnd
	mov rsi,r14;r14d <----------------- b , segundo parAmetro solver_set_bnd
	mov rdx,r13 ;r13 <----------------- x , tercer parAmetro solver_set_bnd
	call solver_set_bnd; 

;fin de ciclo_k:
	inc r15; r15 = r15+1
	jmp .ciclo_k
	
.fin:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    add rsp ,8
    pop rbp
    ret


;*************************************************************************************************


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

; máscara para cambio de signo
movdqu xmm3, [dquad_con_bits_de_signo_float_en_1_y_el_resto_en_0]

.ciclo:

  ; para las filas usamos procesamiento vectorial completo (SIMD)

  ; para las columnas usamos SIMD solo para el cambio de signo, pero la lectura es individual

  call procesar_filas_y_columnas

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


