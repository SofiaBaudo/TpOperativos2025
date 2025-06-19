//Listado de métricas por proceso

#include <metricas.h>

t_metricas listado_metricas;           // Estructura para acumular estadísticas por proceso

void* metricas_proceso(int pid, tipo_metrica metrica){
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