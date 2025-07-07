#include "memoria_fisica.h"

// Variables globales del módulo
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
   
    log_debug(logger_memoria, "Memoria física inicializada exitosamente");
    return true;
}

// Devuelve el número del siguiente marco libre, o -1 si no hay disponibles
int obtener_siguiente_marco_libre(void) {
    int marco_libre = -1;
    pthread_mutex_lock(&memoria_usuario_mutex);
    for (int i = 0; i < cantidad_marcos; i++) {
        if (!marcos[i].ocupado) {
            marco_libre = i;
            break;
        }
    }
    pthread_mutex_unlock(&memoria_usuario_mutex);
    return marco_libre;
}

// Devuelve la cantidad de marcos libres actualmente en memoria
int contar_marcos_libres(void) {
    int libres = 0;
    pthread_mutex_lock(&memoria_usuario_mutex);
    for (int i = 0; i < cantidad_marcos; i++) {
        if (!marcos[i].ocupado) libres++;
    }
    pthread_mutex_unlock(&memoria_usuario_mutex);
    return libres;
}

// Libera un marco físico, marcándolo como libre y reseteando sus datos
void liberar_marco(int nro_marco) {
    if (nro_marco < 0 || nro_marco >= cantidad_marcos)
        return;
    pthread_mutex_lock(&memoria_usuario_mutex);
    marcos[nro_marco].ocupado = false;
    marcos[nro_marco].pid_propietario = -1;
    // Cheuqear: limpiar la memoria física si lo deseas
    pthread_mutex_unlock(&memoria_usuario_mutex);
}

// Lee memoria física en un buffer seguro. Devuelve 0 OK, -1 error.
int leer_memoria_fisica(uint32_t direccion_fisica, void* buffer, size_t tamanio) {
    int ret = 0;
    pthread_mutex_lock(&memoria_usuario_mutex);
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
        ret = -1;
    } else {
        memcpy(buffer, (char*)memoria_usuario + direccion_fisica, tamanio);
    }
    pthread_mutex_unlock(&memoria_usuario_mutex);
    return ret;
}

// Escribe en memoria física desde un buffer. Devuelve 0 OK, -1 error.
int escribir_memoria_fisica(uint32_t direccion_fisica, void* buffer, size_t tamanio) {
    int ret = 0;
    pthread_mutex_lock(&memoria_usuario_mutex);
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
        ret = -1;
    } else {
        memcpy((char*)memoria_usuario + direccion_fisica, buffer, tamanio);
    }
    pthread_mutex_unlock(&memoria_usuario_mutex);
    return ret;
}