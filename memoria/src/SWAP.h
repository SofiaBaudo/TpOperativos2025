#ifndef SWAP_H
#define SWAP_H

#include <bibliotecas.h>
#include <variables_globales_memoria.h>
#include <servidor_memoria.h>
#include <metricas.h>
#include <paginacion.h>

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
void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido,int tamanio_proceso,int cliente);
void* leer_pagina_de_swap(int pid, int nro_pagina, int tamanio_proceso,int cliente);
void eliminar_paginas_de_proceso(int pid);
void cerrar_swap();

// Funciones auxiliares para manejo completo de procesos
void suspender_proceso_completo(int pid);
bool reanudar_proceso_desde_swap(int pid, int tam_proceso);
void escribir_pagina_a_swap_callback(int nro_pagina, int nro_marco, void* contexto);

// Funciones auxiliares para manejar tablas de proceso
t_tabla_proceso* obtener_tabla_proceso(int pid);
void eliminar_tabla_proceso(int pid);
int obtener_marco_de_pagina(t_tabla_paginas* tabla_raiz, int nro_pagina);
void recorrer_paginas_proceso(t_tabla_paginas* tabla_raiz, int nivel, int offset_pagina, void (*callback)(int, int, void*), void* contexto);

#endif