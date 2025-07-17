#include "memoria_fisica.h"

void* memoria_usuario = NULL;

t_marco* marcos = NULL; //represento la memoria con un void*
int cantidad_marcos;
int tam_pagina;

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
    memset(memoria_usuario, 0, memoria_config.TAM_MEMORIA);
   
    // PASO 3: Crear array de marcos --> BITMAPPPP visto en clase
    marcos = malloc(cantidad_marcos * sizeof(t_marco));
    if (marcos == NULL) {
        log_error(logger_memoria, "Error al crear array de marcos");
        free(memoria_usuario);  // Cleanup si falla
        return false;
    }
   
    // PASO 4: Inicializar cada marco
    for (int i = 0; i < cantidad_marcos; i++) {
        marcos[i].numero_marco = i;
        log_debug(logger_memoria, "EL NUM DE marco es %i", marcos[i].numero_marco);
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
    tam_pagina = memoria_config.TAM_PAGINA;
    cantidad_marcos = memoria_config.TAM_MEMORIA / tam_pagina;
   
    pthread_mutex_lock(&memoria_usuario_mutex);
    for (int i = 0; i < cantidad_marcos; i++) {
        if (!marcos[i].ocupado) {
            marco_libre = i;
            marcos[i].ocupado = true; //SE PODRIA GUARDAR TAMBIEN EL PROPIETARIO DEL MARCO
            break;
        }
    }
    pthread_mutex_unlock(&memoria_usuario_mutex);
    return marco_libre;
}

// Devuelve la cantidad de marcos libres actualmente en memoria
int contar_marcos_libres(void) {
    tam_pagina = memoria_config.TAM_PAGINA;
    cantidad_marcos = memoria_config.TAM_MEMORIA / tam_pagina;
   
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
    tam_pagina = memoria_config.TAM_PAGINA;
    cantidad_marcos = memoria_config.TAM_MEMORIA / tam_pagina;
   
    if (nro_marco < 0 || nro_marco >= cantidad_marcos)
        return;
    pthread_mutex_lock(&memoria_usuario_mutex);
    marcos[nro_marco].ocupado = false;
    marcos[nro_marco].pid_propietario = -1;
    // Cheuqear: limpiar la memoria física si lo deseas
    pthread_mutex_unlock(&memoria_usuario_mutex);
}

// Lee memoria física en un buffer seguro. Devuelve 0 OK, -1 error.
int leer_memoria_fisica(int direccion_fisica, char* valor_leido, size_t tamanio) {
    int ret = 0;
    pthread_mutex_lock(&memoria_usuario_mutex);
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
        memset(valor_leido, 0, tamanio);
        ret = -1;
    } else {
        memcpy(valor_leido, (char*)memoria_usuario + direccion_fisica, tamanio);
    }
    pthread_mutex_unlock(&memoria_usuario_mutex);
    return ret;
}

// Lee un marco de memoria física y devuelve un puntero al buffer con su contenido
void* leer_marco_memoria(int nro_marco) {
    void* buffer = malloc(memoria_config.TAM_PAGINA);
    if (!buffer) {
        return NULL;
    }
    memset(buffer, 0, memoria_config.TAM_PAGINA);
    int direccion_fisica = marcos[nro_marco].numero_marco * memoria_config.TAM_PAGINA;
    leer_memoria_fisica(direccion_fisica, buffer, memoria_config.TAM_PAGINA);
    return buffer;
}


// Escribe en memoria física desde un buffer. Devuelve 0 OK, -1 error.
int escribir_memoria_fisica(int direccion_fisica, char* buffer, size_t tamanio) {
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

// Escribe los datos de una página en el marco físico indicado
int escribir_marco_memoria(int nro_marco, void* datos_pagina) {
    tam_pagina = memoria_config.TAM_PAGINA;
    cantidad_marcos = memoria_config.TAM_MEMORIA / tam_pagina;
   
    if (nro_marco < 0 || nro_marco >= cantidad_marcos) {
        log_error(logger_memoria, "escribir_marco_memoria: número de marco inválido %d", nro_marco);
        return -1;
    }
    int direccion_fisica = marcos[nro_marco].numero_marco * memoria_config.TAM_PAGINA;
    return escribir_memoria_fisica(direccion_fisica, datos_pagina, memoria_config.TAM_PAGINA);
}

// Devuelve una lista de números de marcos asignados al proceso con el PID dado
// Función recursiva auxiliar
void recolectar_marcos_tabla(t_tabla_paginas* tabla, int nivel_actual, t_list* lista_marcos) {
    int nivel_maximo = memoria_config.CANTIDAD_NIVELES;
    if (tabla == NULL) return;
    for (int i = 0; i < tabla->cantidad_entradas; i++) {
        t_entrada_tabla* entrada = &tabla->entradas[i];
        if (nivel_actual == nivel_maximo) { // Último nivel
            if( entrada->tabla_nivel_inferior != NULL){
                log_error(logger_memoria, "Error: Entrada de tabla de último nivel con tabla inferior no debería existir");
                continue; // No debería haber tablas inferiores en el último nivel
            }
            int* marco = malloc(sizeof(int));
            *marco = entrada->nro_marco;
            list_add(lista_marcos, marco);
        } else {
            if (entrada->tabla_nivel_inferior != NULL) {
                recolectar_marcos_tabla(entrada->tabla_nivel_inferior, nivel_actual + 1, lista_marcos);
            }
        }
    }
}

t_list* obtener_marcos_proceso(int pid) {
    t_list* lista_marcos = list_create();
    if (lista_marcos == NULL) {
        log_error(logger_memoria, "Error al crear lista de marcos para el proceso %d", pid);
        return NULL;
    }

    t_proceso_memoria* proc = buscar_proceso_en_memoria(pid); // Debe estar implementada
    if (proc == NULL || proc->tabla_paginacion_raiz == NULL) {
        log_error(logger_memoria, "No se encontró el proceso %d o su tabla de páginas", pid);
        list_destroy(lista_marcos);
        return NULL;
    }

    recolectar_marcos_tabla(proc->tabla_paginacion_raiz, 1, lista_marcos);
    return lista_marcos;
}


