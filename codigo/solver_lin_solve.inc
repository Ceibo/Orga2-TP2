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
  mov rbp, rsp
  sub rsp, 8
  push rbx; alineada
  push r12
  push r13; alineada
  push r14
  push r15; alineada
  
;backups de argumentos iniciales
  mov rbx, rdi ;               ** rbx <--- solver **
  mov r12, rcx ;               ** r12 <--- x0 **
  mov r13, rdx ;               ** r13 <--- x **
  mov r14d, esi ;              ** r14d <--- b **
  movss [rbp+offset_a], xmm0 ; ** rbp+offset_a <--- backup de a **
  movss [rbp+offset_c], xmm1 ; ** rbp+offset_c <--- backup de c **
  mov r15, 20
    
.ciclo_k: ; ciclo externo que itera sobre k desde 0 hasta 19
  mov r8, r13 ;                                 ** r8 -> x 
  mov r9, r12 ;                                 ** r9 -> x0
  mov eax, [rbx+offset_N] ;                     ** rax = N
  mov r10d, eax ;                               ** r10 = N
  
  inc rax ;                                     ** rax = N+1
  sal rax, 2 ;                                  ** rax = (N+1)*4
  mov rcx, rax ;                                ** rcx = (N+1)*4
  
  mov rax, r10 ;                                ** eax = N
  sar rax, 2 ;                                  ** rax = N/4

  mov esi, eax ;                                ** esi backup de cociente
  inc eax ;                                     ** eax = (N/4) + 1
  inc r10 ;                                     ** r10 = N+1
  add r8, float_size ;                          ** r8 -> x(1,0)
  add r9, rcx ;                                 ** r9 -> x0(N+1, 0)
  add r9, 8;  r9 apunta a (N+3) Esima posiciOn   ** r9 apunta a x0(1,1)
  
  movss xmm0,[rbp+offset_a] ;                   ** xmm0 = a
  movss xmm1,[rbp+offset_c] ;                   ** xmm1 = c

;guarda de condicional: 
  test esi, esi ; equivale a cmp esi, 0
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
   
   mov edx,r11d; rdx = i
   sal rdx,4 ; rdx = i*16
   add r8,rdx; r8 incrementado en i posiciones
   add r9,rdx; r9 incrementado en i posiciones
   inc r11d; incrementamos i
   jmp .ciclo_principal
   
.seguir:

; llamar función solver_set_bnd:
  mov rdi, rbx  ; rbx  <--- solver, primer parámetro solver_set_bnd
  mov esi, r14d ; r14d <--- b, segundo parámetro solver_set_bnd
  mov rdx, r13  ; r13  <--- x, tercer parámetro solver_set_bnd
  call solver_set_bnd;            

;fin de ciclo_k:
  dec r15 ; r15 = r15-1
  test r15, r15 ; equivale a cmp r15, 0 pero ocupa menos memoria
    jne .ciclo_k
  
.fin:
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    add rsp ,8
    pop rbp
    ret