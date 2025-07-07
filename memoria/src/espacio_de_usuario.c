//Un espacio contiguo de memoria (representado por un void*). Este representara el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir.
#include <espacio_de_usuario.h>

void* memoria_usuario = NULL;
t_marco* marcos = NULL;
int cantidad_marcos = 0;
int tam_pagina = 0;

bool inicializar_memoria_fisica(void) {
    // PASO 1: Calcular parámetros
    tam_pagina = memoria_config.TAM_PAGINA;
    cantidad_marcos = memoria_config.TAM_MEMORIA / tam_pagina;

    // PASO 2: Asignar espacio físico total
    memoria_usuario = malloc(memoria_config.TAM_MEMORIA);
    if (memoria_usuario == NULL) {
        log_error(logger_memoria, "Error al asignar memoria física");
        return false;
    }

    // PASO 3: Crear array de marcos
    marcos = malloc(cantidad_marcos * sizeof(t_marco));
    if (marcos == NULL) {
        log_error(logger_memoria, "Error al crear array de marcos");
        free(memoria_usuario);  // Cleanup si falla
        return false;
    }

    // PASO 4: Inicializar cada marco
    for (int i = 0; i < cantidad_marcos; i++) {
        marcos[i].numero_marco = i;
        marcos[i].pid_propietario = -1;  // Libre
        marcos[i].ocupado = false;
        marcos[i].direccion_fisica = memoria_usuario + (i * tam_pagina);
    }

    log_info(logger_memoria, "Memoria física inicializada exitosamente");
    return true;
}
char* leer_pagina_completa(unsigned int direccion_fisica) {
    if (direccion_fisica + memoria_config.TAM_PAGINA > memoria_config.TAM_MEMORIA)
        return NULL;

    char* buffer = malloc(memoria_config.TAM_PAGINA);
    memcpy(buffer, espacio_usuario_memoria + direccion_fisica, memoria_config.TAM_PAGINA);
    return buffer;
}
char* actualizar_pagina_completa(unsigned int direccion_fisica, char* contenido){
    if (direccion_fisica + memoria_config.TAM_PAGINA > memoria_config.TAM_MEMORIA)
        return "Error: fuera de rango";

    memcpy(espacio_usuario_memoria + direccion_fisica, contenido, memoria_config.TAM_PAGINA);
    return "OK";
}
void leer_espacio_usuario(void* destino, int direccion_fisica, int tamanio){
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
        log_error(logger_memoria, "Lectura fuera de los limites de memoria");
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
    tamanio_disponible_en_memoria = tamanio_disponible_en_memoria - tamanio;
    listado_metricas.cant_escrituras_memoria++;
}

// Función para obtener el siguiente marco libre
int obtener_siguiente_marco_libre(void) {
    for (int i = 0; i < cantidad_marcos; i++) {
        if (!marcos[i].ocupado) {
            marcos[i].ocupado = true;
            return i;
        }
    }
    return -1; // No hay marcos libres
}