#ifndef SWAP_H
#define SWAP_H

#include <bibliotecas.h>
#include <variables_globales_memoria.h>
#include <servidor_memoria.h>
#include <metricas.h>
#include <paginacion.h>
#include <comunicaciones_memoria.h>
#include <memoria_fisica.h>

// Estructura para representar un proceso con su tabla de páginas
typedef struct {
    int pid;
    int tamanio;
    t_tabla_paginas* tabla_raiz;
} t_tabla_proceso;

typedef struct {
    int pid;
    int nro_pagina;// nro de pag logica dentro del proceso
    int offset_en_archivo;// posicion dentro del archivo swapfile.bin
} t_pagina_en_swap;

// Estructura para pasar contexto durante suspensión
typedef struct {
    int pid;
    int cliente;
    bool error_encontrado;
} t_contexto_suspension;

// Funciones principales de SWAP
void inicializar_swap();
void inicializar_lista_procesos();
void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido, int tamanio_proceso, int cliente);
void* leer_pagina_de_swap(int pid, int nro_pagina, int tamanio_proceso, int cliente);
void eliminar_paginas_de_proceso(int pid);
void cerrar_swap();

// Funciones auxiliares para manejar tablas de proceso
t_tabla_proceso* obtener_tabla_proceso(int pid);
void eliminar_tabla_proceso(int pid);
int obtener_marco_de_pagina(t_tabla_paginas* tabla_raiz, int nro_pagina);
void recorrer_paginas_proceso(t_tabla_paginas* tabla_raiz, int nivel, int offset_pagina, void (*callback)(int, int, void*), void* contexto);

// Funciones para manejo de lista de procesos
void agregar_proceso_a_lista(int pid, int tamanio, t_tabla_paginas* tabla_raiz);

// Funciones para manejo de suspensión/reanudación desde kernel
void suspender_proceso_desde_kernel(int pid, int cliente);
void reanudar_proceso_desde_kernel(int pid, int tamanio, int cliente);
void escribir_pagina_a_swap_callback(int nro_pagina, int nro_marco, void* contexto);

// Funciones auxiliares adicionales
bool proceso_tiene_paginas_en_swap(int pid);

// Variables globales externas
extern t_list* lista_procesos;
extern FILE* swapfile;
extern t_list* paginas_en_swap;

#endif