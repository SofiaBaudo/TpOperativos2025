//Define de SWAP

#ifndef SWAP_H
#define SWAP_H

//Incluir las librerias

#include <stdio.h>
#include <commons/collections/list.h>
#include <variables_globales_memoria.h>
#include <stdlib.h>
#include <string.h>
#include <servidor_memoria.h>

//Declaracion Pagina de SWAP

typedef struct {
    int pid;                  // Identificador del proceso dueño de la página
    int nro_pagina;           // Número de página lógica dentro del proceso
    int offset_en_archivo;    // Posición (en bytes) dentro del archivo swapfile.bin
} t_pagina_en_swap;

//Declaracion los Prototipos de Funciones

void inicializar_swap();
void escribir_pagina_en_swap(int pid, int nro_pagina, void* contenido);
void* leer_pagina_de_swap(int pid, int nro_pagina);
void eliminar_paginas_de_proceso(int pid);
void cerrar_swap();

#endif