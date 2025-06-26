//Un espacio contiguo de memoria (representado por un void*). Este representara el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir.
#include <espacio_de_usuario.h>

void* espacio_usuario_memoria = NULL;  // Memoria fisica simulada

//Funcion para inicializar Espacio de Usuario

void inicializar_espacio_usuario(){
    espacio_usuario_memoria = malloc(memoria_config.TAM_MEMORIA);
    if (!espacio_usuario_memoria) {
        printf("Error al reservar memoria de usuario\n");
        exit(EXIT_FAILURE);
    }
}

//Funcion que Retorna toda una pagina (segun TAM_PAGINA) a partir de una direccion fisica.

char* leer_pagina_completa(unsigned int direccion_fisica) {
    if (direccion_fisica + memoria_config.TAM_PAGINA > memoria_config.TAM_MEMORIA)
        return NULL;

    char* buffer = malloc(memoria_config.TAM_PAGINA);
    memcpy(buffer, espacio_usuario_memoria + direccion_fisica, memoria_config.TAM_PAGINA);
    return buffer;
}

//Funcion que Copia los datos completos (una pagina) desde contenido al espacio de usuario.

char* actualizar_pagina_completa(unsigned int direccion_fisica, char* contenido){
    if (direccion_fisica + memoria_config.TAM_PAGINA > memoria_config.TAM_MEMORIA)
        return "Error: fuera de rango";

    memcpy(espacio_usuario_memoria + direccion_fisica, contenido, memoria_config.TAM_PAGINA);
    return "OK";
}

//Funcion que Lee el Espacio de usuario segun su tamanio

void leer_espacio_usuario(void* destino, int direccion_fisica, int tamanio){
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
        log_error(logger_memoria, "Lectura fuera de los limites de memoria");
        return;
    }
    memcpy(destino, espacio_usuario_memoria + direccion_fisica, tamanio);
    listado_metricas.cant_lecturas_memoria++;
}

//Funcion que Escribe el Espacio de usuario segun su tamanio

void escribir_espacio_usuario(int direccion_fisica, void* origen, int tamanio){
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
        log_error(logger_memoria, "Escritura fuera de los límites de memoria");
        return;
    }
    memcpy(espacio_usuario_memoria + direccion_fisica, origen, tamanio);
    tamanio_disponible_en_memoria = tamanio_disponible_en_memoria - tamanio;
    listado_metricas.cant_escrituras_memoria++;
}
op_code verificar_si_hay_espacio(int tamanio){
    if(tamanio_total>tamanio_disponible_en_memoria){
        return NO_HAY_ESPACIO_EN_MEMORIA;
    }
    else{
        tamanio_disponible_en_memoria-=tamanio_total;
        return HAY_ESPACIO_EN_MEMORIA;
    }
}