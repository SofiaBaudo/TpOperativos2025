#include "estructura_memoria.h"
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <variables_globales_memoria.h>
#include <inicializar_memoria.h>
#include <math.h>

t_metricas listado_metricas;           // Estructura para acumular estadísticas por proceso
Tabla* tabla_de_paginas_raiz = NULL;   // Raíz del árbol de paginación multinivel
void* espacio_usuario_memoria = NULL;  // Memoria física simulada

//ver con juanma esta funcion
//reserva la memoria física simulada que usarán los procesos

void inicializar_espacio_usuario() {
    espacio_usuario_memoria = malloc(memoria_config.TAM_MEMORIA);
    if (!espacio_usuario_memoria) {
        printf("Error al reservar memoria de usuario\n");
        exit(EXIT_FAILURE);
    }
}

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

//Verifica si la dirección es válida.
//Usa memcpy para copiar el dato al void*.  ----> preguntar a SOPORTE
//aumenta la métrica de escrituras.

char* funcion_escritura_fisica(unsigned int direccion_fisica, char* valor, int tamanio) {
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA)
        return "Error: fuera de rango";

    memcpy(espacio_usuario_memoria + direccion_fisica, valor, tamanio);
    listado_metricas.cant_escrituras_memoria++;
    return "OK";
}

//Valida rango.
//Copia los bytes deseados a un buffer nuevo y lo devuelve.

char* funcion_lectura_fisica(unsigned int direccion_fisica, int tamanio) {
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA)
        return strdup("Error: fuera de rango");

    char* buffer = malloc(tamanio + 1);
    memcpy(buffer, espacio_usuario_memoria + direccion_fisica, tamanio);
    buffer[tamanio] = '\0';
    listado_metricas.cant_lecturas_memoria++;
    return buffer;
}

//Retorna toda una página (según TAM_PAGINA) a partir de una dirección física.
char* leer_pagina_completa(unsigned int direccion_fisica) {
    if (direccion_fisica + memoria_config.TAM_PAGINA > memoria_config.TAM_MEMORIA)
        return NULL;

    char* buffer = malloc(memoria_config.TAM_PAGINA);
    memcpy(buffer, espacio_usuario_memoria + direccion_fisica, memoria_config.TAM_PAGINA);
    return buffer;
}


//Copia los datos completos (una página) desde contenido al espacio de usuario.
char* actualizar_pagina_completa(unsigned int direccion_fisica, char* contenido) {
    if (direccion_fisica + memoria_config.TAM_PAGINA > memoria_config.TAM_MEMORIA)
        return "Error: fuera de rango";

    memcpy(espacio_usuario_memoria + direccion_fisica, contenido, memoria_config.TAM_PAGINA);
    return "OK";
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

///////////////////////////////////////////////////////////////////////////////
void* metricas_proceso(int pid, tipo_metrica metrica) {
    switch(metrica) {
        case ACCESO_TABLA:
            listado_metricas.cant_acceso_tabla_pagina++;
            break;
        case INSTRUCCIONES_SOLICITADAS:
            listado_metricas.instrucciones_solicitadas++;
            break;
        case BAJADAS_SWAP:
            listado_metricas.bajadas_swap++;
            break;
        case SUBIDAS_MEMORIA:
            listado_metricas.cant_subidas_memoria_principal++;
            break;
        case LECTURAS_MEMORIA:
            listado_metricas.cant_lecturas_memoria++;
            break;
        case ESCRITURAS_MEMORIA:
            listado_metricas.cant_escrituras_memoria++;
            break;
        default:
            break;
    }
    return NULL;
}