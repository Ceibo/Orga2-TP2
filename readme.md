Medidor de ticks:

Compilar: make resultados

Generar archivo de mediciones: ./resultados función nombre_del_archivo_de_salida cantidad_de_mediciones tamaño_de_la_matriz

función: es obligatorio. Opciones: solver_set_bnd, solver_lin_solve, solver_project

El resto de parámetros son optativos.

Ejemplo:

./resultados solver_set_bnd mediciones_solver_set_bnd.csv 100 512

Ejecuta 100 veces la función solver_set_bnd con una matriz de tamaño 512 y guarda la cantidad de ticks de cada iteración en el archivo csv

OJO: Si el archivo estaba creado lo borra, tener cuidado porque si le agrego el vaciado de cache cada ejecución puede tardar mucho.

NOTA: Por el momento solo está implementada la medición para solver_set_bnd


