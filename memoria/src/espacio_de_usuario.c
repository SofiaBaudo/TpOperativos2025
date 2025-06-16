//Un espacio contiguo de memoria (representado por un void*). Este representará el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir.
#include <espacio_de_usuario.h>

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