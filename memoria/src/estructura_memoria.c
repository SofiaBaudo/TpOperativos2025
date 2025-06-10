#include "estructura_memoria.h"
#include <commons/collections/list.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <variables_globales_memoria.h>
#include <inicializar_memoria.h>

// Declaracion de Variables Globales

int TAM_PAGINA;
int CANTIDAD_NIVELES;
int TAM_MEMORIA;
char* info_a_escribir;
char* info_a_leer;
int info_a_buscar;
int informacion;
t_metricas listado_metricas;

// Variables de Paginacion Procesos

int tabla_multinivel [3][2];  //Tabla que apunta a las paginas
int pagina_proceso [64][2];   //Las paginas
int paginacion_usuario [64][2];   //Espacio Contiguo de Memoria de Usuario
float tamanio_marco_proceso;      //Marco tamanio Proceso
float tamanio_marco_usuario;      //Marco tamanio Usuario
int tabla_proceso [4][2];         //Tabla de Marcos del Proces

///////////////////////////////////////////////////////////////////////////////////////////////

//Un espacio contiguo de memoria (representado por un void*). Este representará el espacio de usuario de la misma, donde los procesos podrán leer y/o escribir.

void* espacio_usuario(int pid, int pc, char* proceso_lectura_escritura){
    if (strcmp(proceso_lectura_escritura, "WRITE") == 0){
        funcion_escritura(1, info_a_escribir, 10); //lo hardcodeo para q no de error
    } 
    else if (strcmp(proceso_lectura_escritura, "READ") == 0) {
        funcion_lectura(1, 64, 10); //igual q escritura
    } 
    else {
        log_error(logger_memoria, "Error al codificar la informacion");
    }
    return NULL; //cuando quede implementada hay que chequear que devolver
}

char* funcion_escritura(int pagina, char* info_a_escribir, int direccion){
    for(int i = 0; i < 64; i++) {
        if (pagina_proceso[i][0] == pagina && pagina_proceso[i][1] == direccion) {
            return "OK";
        }
    }
    return "No OK";
}

char* funcion_lectura(int pagina, int tamanio, int direccion) {
    static char info_leida[64];

    for (int i = 0; i < 64; i++){
        if (pagina_proceso [i][0] == pagina && pagina_proceso[i][1] == direccion) {
            snprintf(info_leida, sizeof(info_leida), "Dato en [%d][%d]", i, 1);
            return info_leida;
        }
    }

    printf(info_leida, sizeof(info_leida), "0");
    return info_leida;
}

/////////////////////////////////////////////////////////////////////////////////////////////


void inicializar_tabla_multinivel(){
    for(int i = 0; i < CANTIDAD_NIVELES; i++){
        tabla_multinivel[i][0] = i+1;
        tabla_multinivel[i][1] = i+1;
    }
}

void inicializar_paginas(){

}

bool buscar_en_pagina(int info_a_buscar) {
    for (int i = 0; i < 64; i++) {
        if (pagina_proceso[i][1] == info_a_buscar) {
            return true;
        }
    }
    return false;
}


int* leer_pagina(){
    static int contenido_pagina[64];
    for(int i = 0; i < 64; i++){
        contenido_pagina[i] = pagina_proceso[i][1];
    }
    return contenido_pagina;
}


//Acceso a tabla de páginas
/*
int acceso_tabla_paginas(int pagina_proceso)
{
    int num_marco;
    return num_marco;
}
*/

bool actualizar_pagina(int pagina,int info) {
    for (int i = 0; i < 64; i++) {
        if (pagina_proceso[i][0] == pagina && pagina_proceso[i][1] == info) {
            // Hacer algo
            return true;
        }
    }
    return false;
}



/*
void memory_dump(int proceso){

}
*/

///////////////////////////////////////////////////////////////////////////////////////////////

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





/*
// Variables globales
int TAM_PAGINA;
int CANTIDAD_NIVELES;
int TAM_MEMORIA;
float tamanio_marco;
void* espacio_usuario; // Espacio contiguo de memoria de usuario

// Métricas globales por proceso
t_metricas listado_metricas;

// Lista que contiene una tabla raíz por proceso
// Cada elemento es una estructura TablaProceso con PID y raíz del árbol de paginación

// Estructuras auxiliares
// Tabla jerárquica multinivel para el esquema de paginación
// Si es el último nivel, guarda marcos. Si no, guarda punteros a subniveles.
t_list* tablas_por_proceso;

typedef struct TablaNivel {
    bool es_ultimo_nivel;
    union {
        struct TablaNivel** subniveles; // Punteros a tablas hijas
        int* marcos;                     // Arreglo de marcos si es el último nivel
    } datos;
} TablaNivel;

// Asociación entre un proceso (PID) y su tabla raíz
typedef struct {
    int pid;
    TablaNivel* raiz;
} TablaProceso;

// Inicializa la lista de tablas por proceso
void inicializar_estructuras_kernel() {
    tablas_por_proceso = list_create();
}

// Crea una nueva tabla para un nivel dado (puede ser tabla final o intermedia)
TablaNivel* crear_tabla_nivel(int nivel_actual) {
    TablaNivel* tabla = malloc(sizeof(TablaNivel));
    tabla->es_ultimo_nivel = (nivel_actual == CANTIDAD_NIVELES - 1);

    if (tabla->es_ultimo_nivel) {
        tabla->datos.marcos = calloc(ENTRADAS_POR_TABLA, sizeof(int));
        for (int i = 0; i < ENTRADAS_POR_TABLA; i++)
            tabla->datos.marcos[i] = -1; // -1 indica que aún no hay marco asignado
    } else {
        tabla->datos.subniveles = calloc(ENTRADAS_POR_TABLA, sizeof(TablaNivel*));
    }
    return tabla;
}

// Crea la tabla raíz para un nuevo proceso
void crear_tabla_para_proceso(int pid) {
    TablaProceso* nuevo = malloc(sizeof(TablaProceso));
    nuevo->pid = pid;
    nuevo->raiz = crear_tabla_nivel(0);
    list_add(tablas_por_proceso, nuevo);
}

// Busca la estructura TablaProceso de un PID dado
TablaProceso* buscar_tabla_de_pid(int pid) {
    for (int i = 0; i < list_size(tablas_por_proceso); i++) {
        TablaProceso* tp = list_get(tablas_por_proceso, i);
        if (tp->pid == pid) return tp;
    }
    return NULL;
}

// Asigna un marco físico a una dirección lógica (representada como un array de índices por nivel)
// Si no existían las tablas intermedias, las crea
void asignar_marco_si_es_nuevo(int pid, uint32_t* indices, int marco) {
    TablaProceso* proceso = buscar_tabla_de_pid(pid);
    if (!proceso) return;

    TablaNivel* actual = proceso->raiz;

    // Recorre los niveles intermedios creando tablas si no existen
    for (int nivel = 0; nivel < CANTIDAD_NIVELES - 1; nivel++) {
        int idx = indices[nivel];
        if (!actual->datos.subniveles[idx])
            actual->datos.subniveles[idx] = crear_tabla_nivel(nivel + 1);
        actual = actual->datos.subniveles[idx];
    }

    // En el último nivel, asigna el marco
    actual->datos.marcos[indices[CANTIDAD_NIVELES - 1]] = marco;
}

// Retorna el marco físico asociado a una dirección lógica
// Registra un acceso por cada nivel recorrido
int obtener_marco(int pid, uint32_t* indices) {
    TablaProceso* proceso = buscar_tabla_de_pid(pid);
    if (!proceso) return -1;

    TablaNivel* actual = proceso->raiz;

    for (int nivel = 0; nivel < CANTIDAD_NIVELES - 1; nivel++) {
        int idx = indices[nivel];
        if (!actual->datos.subniveles[idx])
            return -1; // Entrada no asignada aún
        actual = actual->datos.subniveles[idx];
        metricas_proceso(pid, ACCESO_TABLA);
        usleep(memoria_config.RETARDO_MEMORIA * 1000);
    }

    return actual->datos.marcos[indices[CANTIDAD_NIVELES - 1]];
}

// Lee desde el marco correspondiente a la dirección lógica y devuelve un puntero al contenido
void* leer_memoria(int pid, uint32_t* indices) {
    int marco = obtener_marco(pid, indices);
    if (marco == -1) return NULL;
    metricas_proceso(pid, LECTURAS_MEMORIA);
    return (void*)((uintptr_t)espacio_usuario + marco * TAM_PAGINA);
}

// Escribe datos en el marco correspondiente a la dirección lógica
bool escribir_memoria(int pid, uint32_t* indices, void* datos, size_t tamanio) {
    void* destino = leer_memoria(pid, indices);
    if (!destino) return false;
    memcpy(destino, datos, tamanio);
    metricas_proceso(pid, ESCRITURAS_MEMORIA);
    return true;
}

// Copia el contenido completo de una página (marco) a un buffer
bool leer_pagina_completa(int pid, int marco, void* buffer) {
    void* origen = (void*)((uintptr_t)espacio_usuario + marco * TAM_PAGINA);
    if (!origen) return false;
    memcpy(buffer, origen, TAM_PAGINA);
    metricas_proceso(pid, LECTURAS_MEMORIA);
    return true;
}

// Escribe una página completa en memoria (desde el byte 0 del marco)
bool escribir_pagina_completa(int pid, int marco, void* datos) {
    void* destino = (void*)((uintptr_t)espacio_usuario + marco * TAM_PAGINA);
    if (!destino) return false;
    memcpy(destino, datos, TAM_PAGINA);
    metricas_proceso(pid, ESCRITURAS_MEMORIA);
    return true;
}


*/