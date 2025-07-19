#ifndef PAGINACION_H
#define PAGINACION_H

#include "bibliotecas.h"
#include "variables_globales_memoria.h"
#include "inicializar_memoria.h"
#include "memoria_fisica.h"
#include "metricas.h"


typedef struct t_tabla_paginas t_tabla_paginas;

typedef struct {
    int nro_pagina;
    int nro_marco; // Solo válido en el último nivel
    t_tabla_paginas* tabla_nivel_inferior; // NULL si es último nivel. Es inferior porque es la parte de abajo de la jerarquía en el arbol (dibujo)
} t_entrada_tabla;  //lo que contiene una entrada de la tabla.

struct t_tabla_paginas {
    t_entrada_tabla* entradas;
    int cantidad_entradas; //tabla de paginas
};

t_tabla_paginas* iniciar_proceso_paginacion(int pid, int tam_proceso);
t_tabla_paginas* crear_tablas_para_proceso(int tam_proceso);
t_tabla_paginas* crear_nivel_tabla(int nivel_actual, int* paginas_restantes);
// Asigna marcos a todas las páginas de último nivel de la estructura multinivel
bool asignar_marcos_a_todas_las_paginas(t_tabla_paginas* tabla, int nivel_actual);
// Asigna un marco físico a una página lógica específica
void destruir_tabla_y_marcos(t_tabla_paginas* tabla, int nivel_actual); //rec de recursivamente
// Libera todos los marcos asignados a las páginas de último nivel
void liberar_marcos_de_tabla(t_tabla_paginas* tabla, int nivel_actual);
int obtener_marco_de_pagina_logica(int pid, int nro_pagina_logica);
void* obtener_contenido_pagina_completa(int marco, int tam_pagina);
bool actualizar_contenido_pagina_completa(int marco, void* contenido, int tam_pagina);

#endif