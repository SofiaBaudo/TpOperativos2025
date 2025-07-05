#ifndef SWAP_H
#define SWAP_H

#include <stdio.h>
#include <commons/collections/list.h>
#include <variables_globales_memoria.h>
#include <stdlib.h>
#include <string.h>
#include <servidor_memoria.h>

typedef struct {
    int pid;
    int nro_pagina;// nro de pag logica dentro del proceso
    int offset_en_archivo;// posicion dentro del archivo swapfile.bin
} t_pagina_en_swap;

void inicializar_swap();
void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido,int tamanio_proceso,int cliente);
void* leer_pagina_de_swap(int pid, int nro_pagina, int tamanio_proceso,int cliente);
void eliminar_paginas_de_proceso(int pid);
void cerrar_swap();

#endif