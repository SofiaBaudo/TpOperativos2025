#ifndef PAGINACION_H
#define PAGINACION_H

#include "bibliotecas.h"
#include "variables_globales_memoria.h"
#include "inicializar_memoria.h"
#include "memoria_fisica.h"

typedef struct {
    int nro_pagina;
    int nro_marco; // Solo válido en el último nivel
    t_tabla_paginas* tabla_nivel_inferior; // NULL si es último nivel. Es inferior porque es la parte de abajo de la jerarquía en el arbol (dibujo)
} t_entrada_tabla;

struct t_tabla_paginas {
    t_entrada_tabla* entradas;
    int cantidad_entradas;
};

// Inicializa la paginación de un proceso: crea las estructuras y asigna marcos
// Devuelve true si tuvo éxito, false si hubo error de memoria
t_tabla_paginas* iniciar_proceso_paginacion(int pid, int tam_proceso);

// Crea la estructura de tablas de páginas multinivel para un proceso dado su tamaño
t_tabla_paginas* crear_tablas_para_proceso(int tam_proceso);

// Crea recursivamente los niveles de tablas de páginas
t_tabla_paginas* crear_nivel_tabla(int nivel_actual, int paginas_restantes);

// Asigna marcos a todas las páginas de último nivel de la estructura multinivel
bool asignar_marcos_a_todas_las_paginas(t_tabla_paginas* tabla, int nivel_actual);

// Asigna un marco físico a una página lógica específica
bool asignar_marco_a_pagina(t_tabla_paginas* tabla_raiz, int nro_pagina_logica, int nro_marco);

// Libera recursivamente la estructura de tablas de páginas
void destruir_tabla_paginas_rec(t_tabla_paginas* tabla, int nivel_actual);

// Libera todos los marcos asignados a las páginas de último nivel
void liberar_marcos_de_tabla(t_tabla_paginas* tabla, int nivel_actual);

#endif