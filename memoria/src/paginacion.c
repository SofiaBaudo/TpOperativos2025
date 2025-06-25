#include <paginacion.h>

//Declaracion de Variables Globales

t_list* lista_tablas_por_proceso;

//Funcion Creacion Tabla de Niveles

tabla_pagina_nivel* crear_tabla_nivel(int nivel_actual){
    tabla_pagina_nivel* tabla = malloc(sizeof(tabla_pagina_nivel)); //aca creo tabla 1
    tabla->nivel = nivel_actual;
    tabla->entradas = malloc(sizeof(entrada_tabla_pagina) * memoria_config.ENTRADAS_POR_TABLA);

    for (int i = 0; i < memoria_config.ENTRADAS_POR_TABLA; i++) {
        entrada_tabla_pagina* entrada = &tabla->entradas[i];
        entrada->nro_entrada = i;
        entrada->nro_marco = -1;
        if (nivel_actual < memoria_config.CANTIDAD_NIVELES - 1) {
            entrada->siguiente_nivel = (void*) crear_tabla_nivel(nivel_actual + 1);
        } else {
            entrada->siguiente_nivel = NULL;
        }
    }
    return tabla;
}

//Funcion Creacion Tabla del Proceso
tabla_pagina_nivel* crear_tablas_proceso() {
    return crear_tabla_nivel(1); // empieza desde el nivel 1
}


//Funcion Agregar Tabla del Proceso nuevo
void agregar_tablas_proceso(int pid){ // agregar un proceso nuevo
    t_tabla_proceso* nuevo = malloc(sizeof(t_tabla_proceso));
    nuevo->pid = pid;
    nuevo->tabla_raiz = crear_tablas_proceso();
    list_add(lista_tablas_por_proceso, nuevo);
}

//Funcion Buscar Tabla del Proceso(POR PID)
tabla_pagina_nivel* buscar_tabla_por_pid(int pid){ //buscar la tabla raíz por PID
    for (int i = 0; i < list_size(lista_tablas_por_proceso); i++) {
        t_tabla_proceso* p = list_get(lista_tablas_por_proceso, i);
        if (p->pid == pid) 
        return p->tabla_raiz;
    }
    return NULL;
}

//Funcion Liberar Tabla del Proceso
void liberar_tablas(tabla_pagina_nivel* tabla){
    if (tabla == NULL) 
    return;
    for (int i = 0; i < memoria_config.ENTRADAS_POR_TABLA; i++) {
        entrada_tabla_pagina* entrada = &tabla->entradas[i];

        // Si hay una tabla enlazada, liberarla recursivamente
        if (entrada->siguiente_nivel != NULL && tabla->nivel < memoria_config.CANTIDAD_NIVELES - 1) {
            liberar_tablas((tabla_pagina_nivel*) entrada->siguiente_nivel);
        }
    }
    // Liberar array de entradas y luego la tabla misma
    free(tabla->entradas);
    free(tabla);
}
//Funcion Eliminar Tabla del Proceso
void eliminar_tabla_proceso(int pid){
    for (int i = 0; i < list_size(lista_tablas_por_proceso); i++) {
        t_tabla_proceso* p = list_get(lista_tablas_por_proceso, i);
        if (p->pid == pid) {
            liberar_tablas(p->tabla_raiz);
            list_remove(lista_tablas_por_proceso, i);
            free(p);
            return;
        }
    }
}
//Funcion Obtener MARCO para CPU
int obtener_marco_final(tabla_pagina_nivel* tabla_raiz, int* indices_niveles) {
    tabla_pagina_nivel* actual = tabla_raiz;
    for (int i = 0; i < memoria_config.CANTIDAD_NIVELES - 1; i++) {
        int idx = indices_niveles[i];
        if (idx < 0 || idx >= memoria_config.ENTRADAS_POR_TABLA) {
            log_error(logger_memoria, "Índice %d fuera de rango en nivel %d", idx, i);
            return -1;
        }
        entrada_tabla_pagina entrada = actual->entradas[idx];
        if (entrada.siguiente_nivel == NULL) {
            log_error(logger_memoria, "Falta enlace a siguiente nivel en nivel %d", i);
            return -2;
        }
        actual = (tabla_pagina_nivel*) entrada.siguiente_nivel;
    }
    // ultimo nivel
    int idx_final = indices_niveles[memoria_config.CANTIDAD_NIVELES - 1];
    if (idx_final < 0 || idx_final >= memoria_config.ENTRADAS_POR_TABLA) {
        log_error(logger_memoria, "Índice final %d fuera de rango", idx_final);
        return -1;
    }
    entrada_tabla_pagina entrada_final = actual->entradas[idx_final];
    return entrada_final.nro_marco;
}
