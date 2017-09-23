setear_punteros_a_la_matriz:

mov r8d, ecx ; r8 = N
add r8, 2 ; r8 = N+2

mov r14, r8
shl r14, 2 ; r14 = (N+2)*float_size
; r14 = tamaño de fila (para usar después con la instrucción add)

lea r9, [rdx + r14 + float_size] ; r9 -> 2da celda de la 2da fila

mov rdi, rdx ; para preservarlo porque mul lo modifica
mov eax, r8d
mul ecx
mov r8d, eax ; r8 = (N+2)*N
mov rdx, rdi

lea r10, [rdx + float_size*r8 + float_size] ; r10 -> 2da celda de la anteúltima fila

add r8, rcx
add r8, 2 ; r8 = (N+2)*(N+1)

lea r11, [rdx + float_size*r8 + float_size] ; r11 -> 2da celda de la última fila

lea r8, [rdx + float_size] ; r8 -> 2da celda de la 1ra fila

lea r13, [rdx + 2*r14 - 2*float_size] ; r13 -> anteúltima celda de la 2da fila
mov r12, r9 ; r12 -> 2da celda de la 2da fila

ret

procesar_filas_y_columnas:

; PROCESAR FILAS

movups xmm0, [r9]  ; segunda fila
movups xmm1, [r10] ; anteúltima fila
cmp esi, 2
jne .end_if_0
  ; en xmm3 hay una máscara que les cambia el signo a los 4 float
  pxor xmm0, xmm3
  pxor xmm1, xmm3
.end_if_0:
movups [r8], xmm0  ; primera fila
movups [r11], xmm1 ; última fila

add r8, xmm_size
add r9, xmm_size
add r10, xmm_size
add r11, xmm_size

; PROCESAR COLUMNAS

; leer columna izquierda

lea r15, [r12 - float_size]
pinsrd xmm0, [r12], 0
add r12, r14
pinsrd xmm0, [r12], 1
add r12, r14
pinsrd xmm0, [r12], 2
add r12, r14
pinsrd xmm0, [r12], 3
add r12, r14

; leer columna derecha

lea rbx, [r13 + float_size]
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
  ; en xmm3 hay una máscara que les cambia el signo a los 4 float
  pxor xmm0, xmm3
  pxor xmm1, xmm3
.end_if_1:

; guardar columna izquierda

movss [r15], xmm0

add r15, r14
psrldq xmm0, float_size
movss [r15], xmm0

add r15, r14
psrldq xmm0, float_size
movss [r15], xmm0

add r15, r14
psrldq xmm0, float_size
movss [r15], xmm0

; guardar columna derecha

movss [rbx], xmm1

add rbx, r14
psrldq xmm1, float_size
movss [rbx], xmm1

add rbx, r14
psrldq xmm1, float_size
movss [rbx], xmm1

add rbx, r14
psrldq xmm1, float_size
movss [rbx], xmm1

ret

procesar_esquinas:

cmp esi, 1
je .poner_cero_en_las_esquinas

cmp esi, 2
je .poner_cero_en_las_esquinas

; b no es ni 1 ni 2

mov eax, [rdx + float_size]
mov [rdx], eax ; esquina superior izquierda

mov eax, [r9]
mov [r8], eax ; esquina superior derecha

mov eax, [r12]
mov [r12 - float_size], eax ; esquina inferior izquierda

mov eax, [r13]
mov [r11], eax ; esquina inferior derecha

jmp .fin

.poner_cero_en_las_esquinas:

xor eax, eax
mov [rdx], eax ; esquina superior izquierda
mov [r8], eax ; esquina superior derecha
mov [r12 - float_size], eax ; esquina inferior izquierda
mov [r11], eax ; esquina inferior derecha

.fin:
ret