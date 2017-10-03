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
dquad_con_cuatro_float_de_valor_un_medio_negativo: DD -0.5, -0.5, -0.5, -0.5
dquad_con_cuatro_float_de_valor_un_medio: DD 0.5, 0.5, 0.5, 0.5
dword_float_valor_uno: DD 1.0
dword_float_valor_cuatro: DD 4.0

section .text

%include "solver_set_bnd.inc"
%include "solver_lin_solve.inc"
%include "solver_project.inc"
