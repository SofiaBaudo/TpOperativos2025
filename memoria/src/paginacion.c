#include "paginacion.h"


// -------------------- FUNCIÓN PRINCIPAL DEL MÓDULO DE PAGINACIÓN -------------------- //

// Crea las estructuras de paginación y asigna los marcos necesarios
// Retorna la tabla de páginas raíz del proceso si tuvo éxito, NULL si hubo error de memoria
t_tabla_paginas* iniciar_proceso_paginacion(int pid, int tam_proceso) {
    t_tabla_paginas* tabla_raiz = crear_tablas_para_proceso(tam_proceso);
    if (!tabla_raiz)
        return NULL;
    if (!asignar_marcos_a_todas_las_paginas(tabla_raiz, 1)) {
        destruir_tabla_paginas_rec(tabla_raiz, 1);
        return NULL;
    }
    return tabla_raiz;
}

// -------------------- FUNCIONES AUXILIARES USADAS POR iniciar_proceso_paginacion --------------------

// Crea la estructura de tablas para un proceso dado su tamaño
t_tabla_paginas* crear_tablas_para_proceso(int tam_proceso) {
    int tam_pagina = memoria_config.TAM_PAGINA;
    int paginas_necesarias = (tam_proceso + tam_pagina - 1) / tam_pagina;
    return crear_nivel_tabla(1, paginas_necesarias);
}

// Función recursiva para crear niveles de tablas
t_tabla_paginas* crear_nivel_tabla(int nivel_actual, int paginas_restantes) {
    int entradas_por_tabla = memoria_config.ENTRADAS_POR_TABLA;
    int cantidad_entradas = (paginas_restantes > entradas_por_tabla) ? entradas_por_tabla : paginas_restantes;
    t_tabla_paginas* tabla = malloc(sizeof(t_tabla_paginas));
    if (!tabla) {
        log_error(logger_memoria, "Error al asignar memoria para la tabla de páginas en nivel %d", nivel_actual);
        return NULL;
    }
    tabla->entradas = calloc(cantidad_entradas, sizeof(t_entrada_tabla));
    tabla->cantidad_entradas = cantidad_entradas;

    for (int i = 0; i < cantidad_entradas; i++) {
        tabla->entradas[i].nro_pagina = i;
        if (nivel_actual == memoria_config.CANTIDAD_NIVELES) {
            tabla->entradas[i].nro_marco = -1;
            tabla->entradas[i].tabla_nivel_inferior = NULL;
        } else if (paginas_restantes > 0) {
            int paginas_en_subarbol = pow(entradas_por_tabla, memoria_config.CANTIDAD_NIVELES - nivel_actual);
            int paginas_a_mapear = (paginas_restantes > paginas_en_subarbol) ? paginas_en_subarbol : paginas_restantes;
            tabla->entradas[i].nro_marco = -1;
            tabla->entradas[i].tabla_nivel_inferior = crear_nivel_tabla(nivel_actual + 1, paginas_a_mapear);
            paginas_restantes -= paginas_a_mapear;
        } else {
            tabla->entradas[i].nro_marco = -1;
            tabla->entradas[i].tabla_nivel_inferior = NULL;
        }
    }
    return tabla;
}

// Asigna marcos a todas las páginas de último nivel de la estructura multinivel
// Devuelve true si tuvo éxito, false si faltan marcos
bool asignar_marcos_a_todas_las_paginas(t_tabla_paginas* tabla, int nivel_actual) {
    if (!tabla) 
        return false;
    if (nivel_actual == memoria_config.CANTIDAD_NIVELES) {
        for (int i = 0; i < tabla->cantidad_entradas; i++) {
            int marco = obtener_siguiente_marco_libre();
            if (marco == -1) 
                return false;
            tabla->entradas[i].nro_marco = marco;
        }
        return true;
    }
    for (int i = 0; i < tabla->cantidad_entradas; i++) {
        if (tabla->entradas[i].tabla_nivel_inferior) {
            if (!asignar_marcos_a_todas_las_paginas(tabla->entradas[i].tabla_nivel_inferior, nivel_actual + 1))
                return false;
        }
    }
    return true;
}

// Asigna un marco físico a una página lógica específica
// Retorna true si tuvo éxito, false si la página no existe
bool asignar_marco_a_pagina(t_tabla_paginas* tabla_raiz, int nro_pagina_logica, int nro_marco) {
    int niveles = memoria_config.CANTIDAD_NIVELES;
    int entradas = memoria_config.ENTRADAS_POR_TABLA;
    t_tabla_paginas* actual = tabla_raiz;
    for (int nivel = 1; nivel < niveles; nivel++) {
        int idx = (nro_pagina_logica / (int)pow(entradas, niveles-nivel)) % entradas;
        if (!actual->entradas[idx].tabla_nivel_inferior) return false;
        actual = actual->entradas[idx].tabla_nivel_inferior;
    }
    int idx_final = nro_pagina_logica % entradas;
    actual->entradas[idx_final].nro_marco = nro_marco;
    return true;
}

// -------------------- FINALIZACIÓN Y LIBERACIÓN DE TABLAS --------------------

// Libera recursivamente la estructura de tablas de páginas y los marcos asociados
void destruir_tabla_paginas_rec(t_tabla_paginas* tabla, int nivel_actual) {
    if (!tabla) 
        return;
    // Primero liberar los marcos de las páginas de último nivel
    liberar_marcos_de_tabla(tabla, nivel_actual);
    for (int i = 0; i < tabla->cantidad_entradas; i++) {
        if (tabla->entradas[i].tabla_nivel_inferior != NULL)
            destruir_tabla_paginas_rec(tabla->entradas[i].tabla_nivel_inferior, nivel_actual + 1);
    }
    free(tabla->entradas);
    free(tabla);
}

// Libera todos los marcos asignados a las páginas de último nivel
void liberar_marcos_de_tabla(t_tabla_paginas* tabla, int nivel_actual) {
    if (!tabla) 
        return;
    if (nivel_actual == memoria_config.CANTIDAD_NIVELES) {
        for (int i = 0; i < tabla->cantidad_entradas; i++) {
            int marco = tabla->entradas[i].nro_marco;
            if (marco != -1) {
                liberar_marco(marco); 
            }
        }
        return;
    }
    for (int i = 0; i < tabla->cantidad_entradas; i++) {
        if (tabla->entradas[i].tabla_nivel_inferior)
            liberar_marcos_de_tabla(tabla->entradas[i].tabla_nivel_inferior, nivel_actual + 1);
    }
}