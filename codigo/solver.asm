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

constante_unmedio_negativo_para_solver: DD -0.5, -0.5, -0.5, -0.5

section .text

%include "solver_set_bnd_functions.inc"

;*************************************************************************************************


global solver_lin_solve;        

;extern solver_set_bnd
 
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
	;cmp rdx ,0
	;je .fin
	;cmp rcx,0
	;je .fin; 
	
	
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
    
.ciclo_k: ; ciclo externo que itera sobre k desde 0 hasta 19
	cmp r15,    20 ;                                     
	je .fin
	cmp rdx ,0
	je .seguir; si x es NULL entonces saltear ciclo
	cmp rcx,0
	je .seguir;  si x0 es NULL entonces saltear ciclo
	
	mov r8,r13; r8 puntero a x 
	mov r9, r12 ; r9 puntero a x0
	xor rdx,rdx
	xor rax,rax
	xor r10,r10
	mov eax,[rdi+offset_N] ; eax = N
	mov r10d,eax;                               ** r10d <--------  N **
	inc eax; eax = N+1
	xor rcx,rcx
	;mov ecx,4
	;mul ecx; edx : eax = (N+1)*4
	;shl rax,32; ** agregado **
    ;shrd rax,rdx,32;** agregado ** rax = edx:eax
    sal rax,2 ; rax = (N+1)*4
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
	 psrldq xmm8,8; xmm8 = .| x(i+1,j+1)+x(i+2,j)+x(i+1,j-1)
	 psrldq xmm4,8; xmm4 = .| x0(i+1,j)
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
 	 movups [r8+rcx+4],xmm2; updateamos 4 puntos consecutivos en siguiente fila de x  
	 add r8,rcx; r8 apunta a N+1 Esima posiciOn 
	 add r8,4 ; r8 apunta a N+2 Esima posiciOn ; actualizado x
	 add r9,rcx; r9 apunta a N+1 Esima posiciOn
	 add r9,4 ; r9 apunta a N+2 Esima posiciOn ; actualizado x0
	 inc esi; incrementamos j
	 jmp .subciclo_4_en_paralelo
	 
.fin_de_ciclo_principal:
	 
	 xor rsi,rsi
	 mov esi,1; j en 1 
	 
	 mov r8,r13; r8 restaurado a x 
	 mov r9, r12 ; r9 restaurado a x0
	 add r8,4;                                                 ** r8 apunta a x(1,0) **
	 add r9,rcx 
	 add r9,8;  r9 apunta a (N+3) Esima posiciOn                 ** r9 apunta a x0(1,1) **
	  
	 xor rdx,rdx
	 mov edx,r11d; rdx = i
	 sal rdx,4 ; rdx = i*16
	 add r8,rdx; r8 incrementado en i posiciones
	 add r9,rdx; r9 incrementado en i posiciones
	 inc r11d; incrementamos i
	 jmp .ciclo_principal
	 
.seguir:

;llamar funciOn solver_set_bnd:
	xor rdi,rdi
	xor rsi,rsi
	xor rdx,rdx
	mov rdi,rbx;rbx <------------ solver , primer parAmetro solver_set_bnd
	mov esi,r14d;r14d <----------------- b , segundo parAmetro solver_set_bnd
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

; máscara que con pxor cambia el signo a cuatro float empaquetados
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

; global solver_project
; solver_project:
; ;void solver_project ( fluid_solver* solver, float * p, float * div )
; ;rdi = fluid_solver* solver
; ;rsi = float* p
; ;rdx = float* div
; push rbx;
; push r12;
; push r13;
; push r14;
; push r15

; mov eax, [rdi + offset_N] ;Tengo en edx el int32 N, tamaño de la matriz
; mov r8d, eax; me guardo el n en r8d
; add r8d, 3; en r8d tengo N+2
; lea rsi, [rsi + r8*float_size] ;VER SI DEJA USAR RSI O HAY QUE USAR UN REGISTRO AUXILIAR
; sub r8d, 3; Vuelvo a tener N en r8d
; mul eax; Toma eax y lo multiplica por lo que hay en eax.
; mov ecx, eax; Para LOOP, pongo (N+2)^2 en ecx
; mov ebx, eax; En ebx guardo (N+2)^2 para preservarlo
; mov eax, [rdi + offset_u]; Pongo en eax el puntero al primer elemento de la matriz u

; mov r10d, [rdi + offset_v]; Pongo en r10d el puntero al primer elemento de la matriz v
; add r10d, float_size; Ahora apunta al (1,0)(columna, fila)
; add r8d, float_size
; add r8d, float_size; Tengo en r8d N+2
; xor r9, r9;
; mov r9d, [rdi + offset_N]; Acá va el N, que está al principio del stuct
; ;add r10d, r8d; Sumo una fila al primer elemento de la matriz v y estoy ahora
; mov r11, rdx; Guardo en r11 el div. Para poder las llamadas de despues 
; mov rax, rsi; Como es void puedo usar el rax, y lo uso para preservar el rsi.



; mov r12, rdi; En r12 salvo el puntero a solver
; mov r13, rsi; En r13 guardo el puntero a p 
; mov r14, rdx; En r14 guardo el puntero a div

; xor rax, rax

; .ciclo1:
; ;solver->u[IX(i+1,j)]-solver->u[IX(i-1,j)]
; add eax, float_size; Agrego uno al ciclo
; movups xmm1, [eax]; Tengo en xmm1 los (i+1,j)
; lea eax, [eax - 2*float_size]
; movups xmm2, [eax]; Ahora tengo en xmm2 los (i-1,j)
; subps xmm1, xmm2; Acá tengo solver->u[IX(i+1,j)]-solver->u[IX(i-1,j)] en xmm1
; lea eax, [eax + 4*float_size]; Me muevo 4 posiciones para volver a ejecutar
; ; en el primer elemento de la "submatriz" que voy a querer recorrer

; movups xmm3, [r10]; Pongo en xmm3 los primeros 4 elementos de la matriz. El (i,j-1)
; lea r10, [r10 + r8*float_size]; Le agrego N+2 a r10d, o sea, bajo una fila
; lea r10, [r10 + r8*float_size]
; movups xmm4, [r10]; Tengo 4 elementos de la matriz pero una fila abajo de los de xmm3. El (i,j+1)
; subps xmm4, xmm3; En xmm4 tengo la diferencia, el resultasdo que quiero
; lea r10d, [r10d + 4*float_size] ;ESTO ME HACE RUIDO

; ;Ahora tengo en xmm1 y xmm4 los resultados de las cuentas de u y v
; addps xmm1, xmm4; Ahora tengo todo en xmm1, entonces xmm4 está libre tambien ahora
; ;Ahora voy a multiplicar xmm1 por -1/2
; mulps xmm1, [constante_unmedio_negativo_para_solver]; Ahora me falta dividirlo por el solver->N (r9d)
; movd xmm6, r9d; Pongo en la derecha supuestamente de xmm6 el N
; pslld xmm6, float_size; shifteo el 
; movd xmm6, r9d;
; pslld xmm6, float_size
; movd xmm6, r9d;
; pslld xmm6, float_size
; movd xmm6, r9d;
; ;Y ahora tengo en xmm6 algo así: [N, N, N, N]
; ;Ahora divido xmm1 por xmm6
; divps xmm1, xmm6;
; movups [rdx], xmm1



; pxor xmm0, xmm0
; movups [rsi], xmm0
; add r9, float_size; Por que voy a usar xmm y agrrar de a 4 floats
; cmp r8d, eax; Comparo el contador con N+2
; jne .fin
; 	xor rax, rax
; 	sub rsi, float_size
; 	sub eax, float_size
; 	sub r10, float_size
; 	sub rdx, float_size

; .fin:
; add rsi, xmm_size;puntero a p
; add eax, xmm_size;puntero a u
; add r10, xmm_size;puntero a v
; add rdx, xmm_size;puntero a div
; loop .ciclo1

; mov rdx, r12; Restauro rdi para tener el puntero a principio de div para cuando llame
; ;a otras funciones ahora.
; mov rsi, r13; Restauro rsi, el float p, para poder volver a usarlo en otras funciones.

; ;solver_set_bnd ( solver, 0, div )//solver = rdi, 0 = esi, div = rdx
; ;rdi y rdx están ok. Solo tengo que pober esi en 0
; xor rsi, rsi
; call solver_set_bnd

; ;solver_set_bnd ( solver, 0, p ). Solver->rdi, 0->esi, p->rdx
; xor rsi, rsi; pongo eci en 0, por las dudas, tendría que ver si solver_set_bnd lo modifica
; mov rdx, r14; Pongo en rdx el original que contenia p, preservado en r14.
; ;Ahora puedo llamar a solver_set_bnd tranquilo
; call set_solver_bnd


; ;solver_lin_solve ( solver, 0, p, div, 1, 4 ) // solver = rdi, 0 = esi(uint32), p = rdx, div = rcx, 1 = xmm0, 4 = xmm1
; ;mov rdi, SOLVER
; mov rdi, r12; Por las dudas restauro rdi (solver) al original
; xor rsi, rsi; Pongo esi en 0, por las dudas
; mov rdx, r13; Por las dudas restauro rdx (p)
; mov rdx, r14; Por las dudas restauro rcx (div)

; ;Esta es otra forma de pasar constantes a los xmm, pero con movd es mas corto.
; ; mov r9d, 1;
; ; mov r10d, 4;
; ; movss xmm0, r9d;
; ; movss xmm1, r10d;
; movd xmm0, 1;
; movd xmm1, 4;
; ;Ahora sí, puedo llamar a solver_lin_solve sin drama
; call solver_lin_solve



; ; FOR_EACH_CELL
; ; solver->u[IX(i,j)] -= 0.5f*solver->N*(p[IX(i+1,j)]-p[IX(i-1,j)]);
; ; solver->v[IX(i,j)] -= 0.5f*solver->N*(p[IX(i,j+1)]-p[IX(i,j-1)]);
; ; END_FOR
; ;x -= 5 es igual a x = x - 5;
; mov rdi, r12; Vuelvo a restaurar rdi
; mov eax, [rdi + offset_u]; Pongo en eax puntero al primer elemento de la matriz u
; mov r10d, [rdi + offset_v]; Pongo en r10d el puntero al primer elemento de la matriz u
; mulps xmm0, constante_unmedio_negativo_para_solver; Pogo en xmm0 el 1/2 que voy a querer usar despues.
; ;En el codigo en C es (1/2), pero acá voy a usar el (-1/2) por que me puedo ahorrar un define haciendo
; ;un add en lugar de un sub, por lo del -= .
; mov esi, [rdi + offset_N]; Pongo en esi (32bits) el N, tamaño de la matriz sin contar los bordes(uint32) 
; add esi, 2; Ahora tengo N+2. El tamaño posta
; add eax, esi; Avanzo una fila en u
; add eax, float_size; Avanzo un casillero de la matriz. Ahora estoy en (1,1)
; add r10d, esi
; add r10d, float_size; Hago lo mismo con v
; mov r8, r13; Guardo en r8 el puntero a P, la matriz que voy a recorrer ahora
; add r8, esi; Avanzo fila en matriz P 
; add r8, float_size; Me pongo en (1,1) en la matriz P
; mov ecx, ebx; Pongo en ecx (N+2)^2, para el LOOP
; ;Ahora ya tengo todo como para entrar al ciclo
; .ciclo2:




; loop .ciclo2

; pop r15
; pop r14
; pop r13
; pop r12
; pop rbx

; ret


