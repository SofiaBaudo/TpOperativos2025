#include "paginacion.h"


// -------------------- FUNCIÓN PRINCIPAL DEL MÓDULO DE PAGINACIÓN -------------------- //

// Crea las estructuras de paginación y asigna los marcos necesarios
// Retorna la tabla de páginas raíz del proceso si tuvo éxito, NULL si hubo error de memoria

t_tabla_paginas* iniciar_proceso_paginacion(int pid, int tam_proceso) {
    t_tabla_paginas* tabla_raiz = crear_tablas_para_proceso(tam_proceso);
    log_warning(logger_memoria, "termine de crear las tablas para el proceso");
    if (!tabla_raiz){
        log_debug(logger_memoria, "es null la tabla de raiz");
        return NULL;
    }     
    if (!asignar_marcos_a_todas_las_paginas(tabla_raiz, 1)) {
        log_debug(logger_memoria, "vacia a asignacion");
        destruir_tabla_paginas_rec(tabla_raiz, 1);
        tabla_raiz = NULL;
    }
    return tabla_raiz;
}

// -------------------- FUNCIONES AUXILIARES USADAS POR iniciar_proceso_paginacion --------------------

// Crea la estructura de tablas para un proceso dado su tamaño
t_tabla_paginas* crear_tablas_para_proceso(int tam_proceso) {
    int tam_pagina = memoria_config.TAM_PAGINA;
    log_debug(logger_memoria,"EL TAMAÑO DEL PROCESO ES: %i",tam_proceso);
    int paginas_necesarias = (tam_proceso + tam_pagina - 1) / tam_pagina;
    log_error(logger_memoria, "LAS PAGINAS NECESARIAS SON: %d", paginas_necesarias);
    return crear_nivel_tabla(1, paginas_necesarias);
}

// Función recursiva para crear niveles de tablas
t_tabla_paginas* crear_nivel_tabla(int nivel_actual, int paginas_restantes) {
    int entradas_por_tabla = memoria_config.ENTRADAS_POR_TABLA;
    log_error(logger_memoria, "LAS ENTRADAS POR TABLA SON: %d", entradas_por_tabla);
    int cantidad_entradas = 0;

    // En el último nivel, no crees más entradas de las necesarias
    if (nivel_actual == memoria_config.CANTIDAD_NIVELES -1 ) {
        log_warning(logger_memoria, "entre al primer if donde el nivel_actual es igaul a la cant niveles -1");
        if(paginas_restantes > entradas_por_tabla)
            cantidad_entradas = entradas_por_tabla;
        else
            cantidad_entradas = paginas_restantes;
        } 
    else {
        cantidad_entradas = entradas_por_tabla;
        log_warning(logger_memoria, "EL NUEVO VALOR DE CANTIDAD_ENTRADAS ES %i", cantidad_entradas);
    }

    log_warning(logger_memoria, "🧮 Nivel %d: cantidad de entradas = %d", nivel_actual, cantidad_entradas);

    t_tabla_paginas* tabla = malloc(sizeof(t_tabla_paginas));
    if (!tabla) {
        log_error(logger_memoria, "❌ malloc falló en nivel %d", nivel_actual);
        return NULL;
    }

    tabla->entradas = calloc(cantidad_entradas, sizeof(t_entrada_tabla));
    if (!tabla->entradas) {
        log_error(logger_memoria, "❌ calloc falló en nivel %d", nivel_actual);
        free(tabla);
        return NULL;
    }

    tabla->cantidad_entradas = cantidad_entradas; //en este caso 0

    if (tabla->cantidad_entradas <= 0 || tabla->cantidad_entradas > entradas_por_tabla) {
        log_error(logger_memoria, "🧨 Valor inválido en cantidad_entradas: %d en nivel %d", tabla->cantidad_entradas, nivel_actual);
    }

    log_debug(logger_memoria, "✅ Tabla creada en nivel %d (%p) con %d entradas (restantes: %d)",nivel_actual, tabla, cantidad_entradas, paginas_restantes);

    for (int i = 0; i < cantidad_entradas; i++) {
        tabla->entradas[i].nro_pagina = i;
        tabla->entradas[i].nro_marco = -1;

        if (nivel_actual == memoria_config.CANTIDAD_NIVELES - 1) {
            if (paginas_restantes > 0) {
                paginas_restantes--;
            }
            tabla->entradas[i].tabla_nivel_inferior = NULL;
        } else {
            if (paginas_restantes > 0) {
                int paginas_en_subarbol = pow(entradas_por_tabla, memoria_config.CANTIDAD_NIVELES - nivel_actual - 1);
                int paginas_a_mapear = paginas_restantes > paginas_en_subarbol ? paginas_en_subarbol : paginas_restantes;

                tabla->entradas[i].tabla_nivel_inferior = crear_nivel_tabla(nivel_actual + 1, paginas_a_mapear);
                if (!tabla->entradas[i].tabla_nivel_inferior) {
                    log_error(logger_memoria, "❌ Error al crear subnivel en entrada %d del nivel %d", i, nivel_actual);

                    for (int j = 0; j < i; j++) {
                        if (tabla->entradas[j].tabla_nivel_inferior) {
                            destruir_tabla_paginas_rec(tabla->entradas[j].tabla_nivel_inferior, nivel_actual + 1);
                            tabla->entradas[j].tabla_nivel_inferior = NULL;
                        }
                    }
                    free(tabla->entradas);
                    free(tabla);
                    return NULL;
                }

                paginas_restantes -= paginas_a_mapear;
            } else {
                tabla->entradas[i].tabla_nivel_inferior = NULL;
            }
        }
    }
    log_debug(logger_memoria, "POR RETONAR TABLA");
    return tabla;
}

// Asigna marcos a todas las páginas de último nivel de la estructura multinivel
bool asignar_marcos_a_todas_las_paginas(t_tabla_paginas* tabla, int nivel_actual) {
    if (!tabla){
        log_debug(logger_memoria, "tabla vacia de asignar marcos");
        return false;
    }
    if (nivel_actual == memoria_config.CANTIDAD_NIVELES) {
        for (int i = 0; i < tabla->cantidad_entradas; i++) {
            int marco = obtener_siguiente_marco_libre();
            if (marco == -1) 
                return false;
            tabla->entradas[i].nro_marco = marco;
        }
        return true;
    }
    for (int i = 0; i < tabla->cantidad_entradas; i++) {
        if (tabla->entradas[i].tabla_nivel_inferior) {
            if (!asignar_marcos_a_todas_las_paginas(tabla->entradas[i].tabla_nivel_inferior, nivel_actual + 1))
                return false;
        }
    }
    log_debug(logger_memoria, "pase toda la funcion, voy a retornar true");
    return true;
}

// Asigna un marco físico a una página lógica específica
bool asignar_marco_a_pagina(t_tabla_paginas* tabla_raiz, int nro_pagina_logica, int nro_marco) {
    int niveles = memoria_config.CANTIDAD_NIVELES;
    int entradas = memoria_config.ENTRADAS_POR_TABLA;
    t_tabla_paginas* actual = tabla_raiz;

    for (int nivel = 1; nivel < niveles; nivel++) {
        log_debug(logger_memoria, "entre al for de asignacion de marcos");
        int idx = (nro_pagina_logica / (int)pow(entradas, niveles-nivel)) % entradas;
        if (!actual->entradas[idx].tabla_nivel_inferior) return false;
        actual = actual->entradas[idx].tabla_nivel_inferior;
    }
    int idx_final = nro_pagina_logica % entradas;
    actual->entradas[idx_final].nro_marco = nro_marco;
    return true;
}

// -------------------- FINALIZACIÓN Y LIBERACIÓN DE TABLAS --------------------

// Libera recursivamente la estructura de tablas de páginas y los marcos asociados
void destruir_tabla_paginas_rec(t_tabla_paginas* tabla, int nivel_actual) {
    if (!tabla) {
        log_warning(logger_memoria, "⚠️ Nivel %d: intento de destruir tabla NULL", nivel_actual);
        return;
    }

    log_debug(logger_memoria, "🗑️  Destruyendo tabla nivel %d en %p con %d entradas",
              nivel_actual, tabla, tabla->cantidad_entradas);

    for (int i = 0; i < tabla->cantidad_entradas; i++) {
        if (tabla->entradas[i].tabla_nivel_inferior != NULL) {
            destruir_tabla_paginas_rec(tabla->entradas[i].tabla_nivel_inferior, nivel_actual + 1);
            tabla->entradas[i].tabla_nivel_inferior = NULL;
        }
    }

    free(tabla->entradas);
    free(tabla);

    log_debug(logger_memoria, "✅ Nivel %d destruido correctamente", nivel_actual);
}


// Libera todos los marcos asignados a las páginas de último nivel
void liberar_marcos_de_tabla(t_tabla_paginas* tabla, int nivel_actual) {
    if (!tabla) 
        return;
    if (nivel_actual == memoria_config.CANTIDAD_NIVELES) {
        for (int i = 0; i < tabla->cantidad_entradas; i++) {
            int marco = tabla->entradas[i].nro_marco;
            if (marco != -1) {
                liberar_marco(marco); 
            }
        }
        return;
    }
    for (int i = 0; i < tabla->cantidad_entradas; i++) {
        if (tabla->entradas[i].tabla_nivel_inferior)
            liberar_marcos_de_tabla(tabla->entradas[i].tabla_nivel_inferior, nivel_actual + 1);
    }
}

// -------------------- FUNCIONES PARA ACCESO A TABLA DE PÁGINAS --------------------

// Obtiene el número de marco de una página lógica navegando la tabla multinivel
// Aplica retardo configurado por cada nivel accedido y actualiza métricas
int obtener_marco_de_pagina_logica(int pid, int nro_pagina_logica) {
    // Buscar el proceso en la lista de procesos
    pthread_mutex_lock(&mutex_procesos_en_memoria);
    
    t_proceso_memoria* proceso = NULL;
    for (int i = 0; i < list_size(procesos_en_memoria); i++) {
        t_proceso_memoria* proc = list_get(procesos_en_memoria, i);
        if (proc->pid == pid) {
            proceso = proc;
            break;
        }
    }
    
    if (!proceso) {
        pthread_mutex_unlock(&mutex_procesos_en_memoria);
        log_error(logger_memoria, "PID %d no encontrado para acceso a tabla de páginas", pid);
        return -1;
    }
    
    t_tabla_paginas* tabla_raiz = (t_tabla_paginas*)proceso->tabla_paginacion_raiz;
    pthread_mutex_unlock(&mutex_procesos_en_memoria);
    
    if (!tabla_raiz) {
        log_error(logger_memoria, "Tabla de páginas no encontrada para PID %d", pid);
        return -1;
    }
    
    // Navegar la tabla multinivel aplicando retardo por cada nivel
    int niveles = memoria_config.CANTIDAD_NIVELES;
    int entradas = memoria_config.ENTRADAS_POR_TABLA;
    t_tabla_paginas* actual = tabla_raiz;
    
    log_debug(logger_memoria, "PID: %d - Acceso a tabla de páginas - Página: %d", pid, nro_pagina_logica);
    
    for (int nivel = 1; nivel < niveles; nivel++) {
        // Aplicar retardo por acceso a tabla
        log_debug(logger_memoria,"Adentro del for");
        usleep(memoria_config.RETARDO_MEMORIA * 1000);
        
        // Actualizar métricas
        actualizar_metricas_acceso_tabla_paginas(pid);
        log_debug(logger_memoria, "termine de actualizar metricas");
        usleep(2000000);
        // Calcular índice para este nivel
        int idx = (nro_pagina_logica / (int)pow(entradas, niveles-nivel)) % entradas;
        
        if (idx >= actual->cantidad_entradas || !actual->entradas[idx].tabla_nivel_inferior) {
            log_error(logger_memoria, "PID: %d - Página %d no encontrada en nivel %d", pid, nro_pagina_logica, nivel);
            return -1;
        }
        
        actual = actual->entradas[idx].tabla_nivel_inferior;
    }
    
    // Acceso final a la página de último nivel
    usleep(memoria_config.RETARDO_MEMORIA * 1000);
    actualizar_metricas_acceso_tabla_paginas(pid);
    
    int idx_final = nro_pagina_logica % entradas;
    if (idx_final >= actual->cantidad_entradas) {
        log_error(logger_memoria, "PID: %d - Página %d fuera de rango en nivel final", pid, nro_pagina_logica);
        return -1;
    }
    
    int marco = actual->entradas[idx_final].nro_marco;
    
    log_debug(logger_memoria, "PID: %d - Acceso a tabla de páginas - Página: %d - Marco: %d", pid, nro_pagina_logica, marco);
    
    return marco;
}

// Obtiene el contenido completo de una página desde la memoria física
void* obtener_contenido_pagina_completa(int marco, int tam_pagina) {
    if (marco < 0 || marco >= cantidad_marcos) {
        log_error(logger_memoria, "Marco %d fuera de rango", marco);
        return NULL;
    }
    
    // Aplicar retardo por acceso a memoria
    usleep(memoria_config.RETARDO_MEMORIA * 1000);
    
    void* contenido = malloc(tam_pagina);
    if (!contenido) {
        log_error(logger_memoria, "Error al asignar memoria para contenido de página");
        return NULL;
    }
    
    // Leer el contenido completo del marco
    pthread_mutex_lock(&memoria_usuario_mutex);
    void* direccion_marco = memoria_usuario + (marco * tam_pagina);
    memcpy(contenido, direccion_marco, tam_pagina);
    pthread_mutex_unlock(&memoria_usuario_mutex);
    
    log_debug(logger_memoria, "Acceso a página completa - Marco: %d - Tamaño: %d", marco, tam_pagina);
    
    return contenido;
}

// Actualiza el contenido completo de una página en la memoria física
bool actualizar_contenido_pagina_completa(int marco, void* contenido, int tam_pagina) {
    if (marco < 0 || marco >= cantidad_marcos) {
        log_error(logger_memoria, "Marco %d fuera de rango", marco);
        return false;
    }
    
    if (!contenido) {
        log_error(logger_memoria, "Contenido nulo para actualizar marco %d", marco);
        return false;
    }
    
    // Aplicar retardo por acceso a memoria
    usleep(memoria_config.RETARDO_MEMORIA * 1000);
    
    // Escribir el contenido completo al marco
    pthread_mutex_lock(&memoria_usuario_mutex);
    void* direccion_marco = memoria_usuario + (marco * tam_pagina);
    memcpy(direccion_marco, contenido, tam_pagina);
    pthread_mutex_unlock(&memoria_usuario_mutex);
    
    log_debug(logger_memoria, "Actualización de página completa - Marco: %d - Tamaño: %d", marco, tam_pagina);
    
    return true;
}