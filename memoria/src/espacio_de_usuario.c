//Un espacio contiguo de memoria (representado por un void*). Este representará el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir.
#include <espacio_de_usuario.h>

void* espacio_usuario_memoria = NULL;  // Memoria física simulada

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
char* actualizar_pagina_completa(unsigned int direccion_fisica, char* contenido){
    if (direccion_fisica + memoria_config.TAM_PAGINA > memoria_config.TAM_MEMORIA)
        return "Error: fuera de rango";

    memcpy(espacio_usuario_memoria + direccion_fisica, contenido, memoria_config.TAM_PAGINA);
    return "OK";
}
void leer_espacio_usuario(void* destino, int direccion_fisica, int tamanio){
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
        log_error(logger_memoria, "Lectura fuera de los límites de memoria");
        return;
    }
    memcpy(destino, espacio_usuario_memoria + direccion_fisica, tamanio);
    listado_metricas.cant_lecturas_memoria++;
}
void escribir_espacio_usuario(int direccion_fisica, void* origen, int tamanio){
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
        log_error(logger_memoria, "Escritura fuera de los límites de memoria");
        return;
    }
    memcpy(espacio_usuario_memoria + direccion_fisica, origen, tamanio);
    listado_metricas.cant_escrituras_memoria++;
}
