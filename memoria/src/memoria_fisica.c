#include "memoria_fisica.h"

void* memoria_usuario = NULL;

t_marco* marcos = NULL; //represento la memoria con un void*
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
int leer_memoria_fisica(int direccion_fisica, char* valor_leido, size_t tamanio) {
    int ret = 0;
    pthread_mutex_lock(&memoria_usuario_mutex);
    if (direccion_fisica + tamanio > memoria_config.TAM_MEMORIA) {
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
    if (!buffer) return NULL;
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
    if (nro_marco < 0 || nro_marco >= cantidad_marcos) {
        log_error(logger_memoria, "escribir_marco_memoria: número de marco inválido %d", nro_marco);
        return -1;
    }
    int direccion_fisica = marcos[nro_marco].numero_marco * memoria_config.TAM_PAGINA;
    return escribir_memoria_fisica(direccion_fisica, datos_pagina, memoria_config.TAM_PAGINA);
}

// Devuelve una lista de números de marcos asignados al proceso con el PID dado
t_list* obtener_marcos_proceso(int pid) {
    t_list* lista_marcos = list_create();
    if (lista_marcos == NULL) {
        log_error(logger_memoria, "Error al crear lista de marcos para el proceso %d", pid);
        return NULL;
    }

    // Buscar el proceso en memoria para acceder a su tabla de páginas
    t_proceso_memoria* proc = buscar_proceso_en_memoria(pid); // Debe estar implementada
    if (proc == NULL || proc->tabla_paginacion_raiz == NULL) {
        log_error(logger_memoria, "No se encontró el proceso %d o su tabla de páginas", pid);
        list_destroy(lista_marcos);
        return NULL;
    }

    t_tabla_paginas* tabla = proc->tabla_paginacion_raiz;
    int niveles = memoria_config.CANTIDAD_NIVELES;
    for (int i = 0; i < tabla->cantidad_entradas; i++) {
        t_entrada_tabla* entrada = &tabla->entradas[i];
        t_entrada_tabla* actual = entrada;
        t_tabla_paginas* tabla_actual = tabla;
        // Bajar exactamente niveles-1 veces para llegar al último nivel
        for (int nivel = 1; nivel < niveles; nivel++) {
            if (actual->tabla_nivel_inferior == NULL) {
                log_error(logger_memoria, "Error: tabla_nivel_inferior NULL antes de llegar al último nivel para página %d", i);
                break;
            }
            tabla_actual = actual->tabla_nivel_inferior;
            actual = &tabla_actual->entradas[actual->nro_pagina];
        }
        // Ahora actual apunta a la entrada del último nivel
        int* nro_marco = malloc(sizeof(int));
        *nro_marco = actual->nro_marco;
        list_add(lista_marcos, nro_marco);
    }
    return lista_marcos;
}
