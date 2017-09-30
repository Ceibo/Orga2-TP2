global solver_project_asm
solver_project_asm:
;void solver_project ( fluid_solver* solver, float * p, float * div )
;rdi = fluid_solver* solver
;rsi = float* p
;rdx = float* divpush rbx;
push r12;
push r13;
push r14;
push r15

mov eax, [rdi + offset_N] ;Tengo en edx el int32 N, tamaño de la matriz
mov r8d, eax; me guardo el n en r8d
add r8d, 3; en r8d tengo N+2
lea rsi, [rsi + r8*float_size] ;VER SI DEJA USAR RSI O HAY QUE USAR UN REGISTRO AUXILIAR
sub r8d, 3; Vuelvo a tener N en r8d
mul eax; Toma eax y lo multiplica por lo que hay en eax.
mov ecx, eax; Para LOOP, pongo (N+2)^2 en ecx
mov ebx, eax; En ebx guardo (N+2)^2 para preservarlo
mov eax, [rdi + offset_u]; Pongo en eax el puntero al primer elemento de la matriz u

mov r10d, [rdi + offset_v]; Pongo en r10d el puntero al primer elemento de la matriz v
add r10d, float_size; Ahora apunta al (1,0)(columna, fila)
add r8d, float_size
add r8d, float_size; Tengo en r8d N+2
xor r9, r9;
mov r9d, [rdi + offset_N]; Acá va el N, que está al principio del stuct
;add r10d, r8d; Sumo una fila al primer elemento de la matriz v y estoy ahora
mov r11, rdx; Guardo en r11 el div. Para poder las llamadas de despues 
mov rax, rsi; Como es void puedo usar el rax, y lo uso para preservar el rsi.



mov r12, rdi; En r12 salvo el puntero a solver
mov r13, rsi; En r13 guardo el puntero a p 
mov r14, rdx; En r14 guardo el puntero a div

xor rax, rax

.ciclo1:
call cuerpo_ciclo1
loop .ciclo1

mov rdx, r12; Restauro rdi para tener el puntero a principio de div para cuando llame
;a otras funciones ahora.
mov rsi, r13; Restauro rsi, el float p, para poder volver a usarlo en otras funciones.

;solver_set_bnd ( solver, 0, div )//solver = rdi, 0 = esi, div = rdx
;rdi y rdx están ok. Solo tengo que pober esi en 0
xor rsi, rsi
call solver_set_bnd

;solver_set_bnd ( solver, 0, p ). Solver->rdi, 0->esi, p->rdx
xor rsi, rsi; pongo eci en 0, por las dudas, tendría que ver si solver_set_bnd lo modifica
mov rdx, r14; Pongo en rdx el original que contenia p, preservado en r14.
;Ahora puedo llamar a solver_set_bnd tranquilo
call solver_set_bnd


;solver_lin_solve ( solver, 0, p, div, 1, 4 ) // solver = rdi, 0 = esi(uint32), p = rdx, div = rcx, 1 = xmm0, 4 = xmm1
;mov rdi, SOLVER
mov rdi, r12; Por las dudas restauro rdi (solver) al original
xor rsi, rsi; Pongo esi en 0, por las dudas
mov rdx, r13; Por las dudas restauro rdx (p)
mov rdx, r14; Por las dudas restauro rcx (div)

;Esta es otra forma de pasar constantes a los xmm, pero con movd es mas corto.
mov r9d, 1;
mov r10d, 4;
movd xmm0, r9d;
movd xmm1, r10d;
;Ahora sí, puedo llamar a solver_lin_solve sin drama
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

pop r15
pop r14
pop r13
pop r12
pop rbx

ret




cuerpo_ciclo1:
;solver->u[IX(i+1,j)]-solver->u[IX(i-1,j)]
add eax, float_size; Agrego uno al ciclo
movups xmm1, [eax]; Tengo en xmm1 los (i+1,j)
lea eax, [eax - 2*float_size]
movups xmm2, [eax]; Ahora tengo en xmm2 los (i-1,j)
subps xmm1, xmm2; Acá tengo solver->u[IX(i+1,j)]-solver->u[IX(i-1,j)] en xmm1
lea eax, [eax + 4*float_size]; Me muevo 4 posiciones para volver a ejecutar
; en el primer elemento de la "submatriz" que voy a querer recorrer

movups xmm3, [r10]; Pongo en xmm3 los primeros 4 elementos de la matriz. El (i,j-1)
lea r10, [r10 + r8*float_size]; Le agrego N+2 a r10d, o sea, bajo una fila
lea r10, [r10 + r8*float_size]
movups xmm4, [r10]; Tengo 4 elementos de la matriz pero una fila abajo de los de xmm3. El (i,j+1)
subps xmm4, xmm3; En xmm4 tengo la diferencia, el resultasdo que quiero
lea r10d, [r10d + 4*float_size] ;ESTO ME HACE RUIDO

;Ahora tengo en xmm1 y xmm4 los resultados de las cuentas de u y v
addps xmm1, xmm4; Ahora tengo todo en xmm1, entonces xmm4 está libre tambien ahora
;Ahora voy a multiplicar xmm1 por -1/2
mulps xmm1, [constante_unmedio_negativo_para_solver]; Ahora me falta dividirlo por el solver->N (r9d)
movd xmm6, r9d; Pongo en la derecha supuestamente de xmm6 el N
pslld xmm6, float_size; shifteo el 
movd xmm6, r9d;
pslld xmm6, float_size
movd xmm6, r9d;
pslld xmm6, float_size
movd xmm6, r9d;
;Y ahora tengo en xmm6 algo así: [N, N, N, N]
;Ahora divido xmm1 por xmm6
divps xmm1, xmm6;
movups [rdx], xmm1



pxor xmm0, xmm0
movups [rsi], xmm0
add r9, float_size; Por que voy a usar xmm y agrrar de a 4 floats
cmp r8d, eax; Comparo el contador con N+2
jne .fin
	xor rax, rax
	sub rsi, float_size
	sub eax, float_size
	sub r10, float_size
	sub rdx, float_size

.fin:
add rsi, xmm_size;puntero a p
add eax, xmm_size;puntero a u
add r10, xmm_size;puntero a v
add rdx, xmm_size;puntero a div
ret
