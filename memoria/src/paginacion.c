#include <paginacion.h>
//Las tablas de páginas, que representarán el espacio de Kernel.

Tabla* tabla_de_paginas_raiz = NULL;   // Raíz del árbol de paginación multinivel

/*
estructura tipo árbol con cantidad_niveles de profundidad.
Si está en el último nivel: reserva un array de marcos (valores).
Si está en niveles intermedios: crea sub-tablas recursivamente.
*/

Tabla* crear_tabla(int nivel_actual, int cantidad_niveles, int entradas_por_tabla) {
    if (cantidad_niveles == 0 || entradas_por_tabla == 0) return NULL;

    Tabla* tabla = malloc(sizeof(Tabla));

    if (nivel_actual == cantidad_niveles - 1) {
        tabla->valores = malloc(sizeof(int) * entradas_por_tabla);
        tabla->punteros = NULL;
        for (int i = 0; i < entradas_por_tabla; i++) {
            tabla->valores[i] = rand() % (memoria_config.TAM_MEMORIA / memoria_config.TAM_PAGINA); // marco aleatorio
        }
    } else {
        tabla->valores = NULL;
        tabla->punteros = malloc(sizeof(Tabla*) * entradas_por_tabla);
        for (int i = 0; i < entradas_por_tabla; i++) {
            tabla->punteros[i] = crear_tabla(nivel_actual + 1, cantidad_niveles, entradas_por_tabla);
        }
    }
    return tabla;
}

//recorre recursivamente las sub-tablas y libera cada una. Se llama al final o al destruir un proceso.
void liberar_tabla(Tabla* tabla, int nivel_actual, int cantidad_niveles, int entradas_por_tabla) {
    if (!tabla) return;

    if (nivel_actual == cantidad_niveles - 1) {
        free(tabla->valores);
    } else {
        for (int i = 0; i < entradas_por_tabla; i++) {
            liberar_tabla(tabla->punteros[i], nivel_actual + 1, cantidad_niveles, entradas_por_tabla);
        }
        free(tabla->punteros);
    }
    free(tabla);
}

int obtener_marco(int direccion_logica, int tam_pagina, int cantidad_niveles, int entradas_por_tabla, Tabla* tabla) {
    int nro_pagina = direccion_logica / tam_pagina;

    Tabla* actual = tabla;
    for (int nivel = 0; nivel < cantidad_niveles - 1; nivel++) {
        int entrada = (nro_pagina / (int)pow(entradas_por_tabla, cantidad_niveles - 1 - nivel)) % entradas_por_tabla;
        actual = actual->punteros[entrada];
    }

    int entrada_final = nro_pagina % entradas_por_tabla;
    int marco = actual->valores[entrada_final];
    listado_metricas.cant_acceso_tabla_pagina++;
    return marco;
}