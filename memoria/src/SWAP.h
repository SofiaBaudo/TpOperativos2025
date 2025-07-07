#ifndef SWAP_H
#define SWAP_H

#include <variables_globales_memoria.h>
#include <servidor_memoria.h>
#include <metricas.h>
#include <paginacion.h>
#include <espacio_de_usuario.h>
#include <bibliotecas.h>

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

void inicializar_swap();
void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido,int tamanio_proceso,int cliente);
void* leer_pagina_de_swap(int pid, int nro_pagina, int tamanio_proceso,int cliente);
void eliminar_paginas_de_proceso(int pid);
void cerrar_swap();

// Funciones auxiliares para manejo completo de procesos
void suspender_proceso_completo(int pid);
bool reanudar_proceso_desde_swap(int pid, int tam_proceso);
void escribir_pagina_a_swap_callback(int nro_pagina, int nro_marco, void* contexto);

#endif